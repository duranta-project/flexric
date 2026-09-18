#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

# Generate the FlexRIC CI HTML report (test_results_oai_flexric.html).

import argparse
import datetime
import html
import os
import re
import string
import sys
import traceback

REPORT_NAME = 'test_results_oai_flexric.html'

# Resolved against this file, not the working directory: the pipeline invokes the
# script as ./ci-scripts/generateFlexricReport.py from the workspace root.
TEMPLATE_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                             'report-template.html')

UBUNTU_LOG = 'flexric_ubuntu_image_build.log'
EL_LOG = 'flexric_el_image_build.log'

# One chapter per sanitizer/E2AP/KPM combination the CTests stage builds.
CTEST_LOGS = [
    ('flexric_ctests_none_e2ap_v1_kpm_v2_01.log',
     'CTest: NONE sanitizer & E2AP_V1 & KPM_V2_01'),
    ('flexric_ctests_address_e2ap_v2_kpm_v2_03.log',
     'CTest: ADDRESS sanitizer & E2AP_V2 & KPM_V2_03'),
    ('flexric_ctests_thread_e2ap_v3_kpm_v3_00.log',
     'CTest: THREAD sanitizer & E2AP_V3 & KPM_V3_00'),
]

# gcc diagnostic, optionally behind a BuildKit "#21 2.054 " progress prefix.
WARNING_RE = re.compile(
    r'^(?:#\d+\s+[\d.]+\s+)?(/\S+?):(\d+):\d+:\s+(warning|error):\s*(.*)$')

# ctest result line: " 1/12 Test  #6: Unit_test_MAC_SM ....   Passed    0.01 sec"
CTEST_RESULT_RE = re.compile(
    # ctest pads with dots and puts NO space before a ***Failed marker, unlike Passed.
    r'Test\s+#\d+:\s+(\S+)\s+\.+\s*(Passed|\*\*\*Failed|\*\*\*Timeout|\*\*\*Exception|Failed)')
CTEST_SUMMARY_RE = re.compile(r'(\d+)% tests passed,\s+(\d+) tests failed out of\s+(\d+)')

# Shown wherever a value could not be determined.
MISSING = '-'

# BuildKit and podman colour their output. Strip both real escape sequences and the
# bare "[33m" remnants left behind when the log is captured without a TTY.
ANSI_RE = re.compile(r'\x1b\[[0-9;]*[A-Za-z]|(?<![0-9A-Za-z])\[[0-9]{1,3}m')

# Marker the pipeline appends per architecture. buildx never reports per-platform
# image sizes itself, so the pipeline has to state them for these cells to fill in.
ARCH_SIZE_RE = re.compile(r'IMAGE SIZE\s+(\S+)\s*:\s*([0-9.]+\s*[KMGT]?i?B)', re.IGNORECASE)


def strip_ansi(text):
    return ANSI_RE.sub('', text)


def read_log(path):
    """Return the log as a list of clean lines, or None when it does not exist."""
    if not os.path.isfile(path):
        return None
    with open(path, 'r', encoding='utf-8', errors='replace') as logfile:
        return [strip_ansi(line.rstrip('\n')) for line in logfile]


def normalize_size(size):
    """'137MB' -> '137 MB'."""
    return re.sub(r'([0-9.])\s*([KMGT]?i?B)$', r'\1 \2', size.strip())


def short_image_ref(ref):
    """Drop the implicit Docker Hub prefix so a base image reads as 'ubuntu:noble'."""
    return re.sub(r'^docker\.io/(library/)?', '', ref)


def has_tag(ref):
    """True when the reference carries a :tag. A registry port or digest is not one."""
    return ':' in ref.split('@', 1)[0].rsplit('/', 1)[-1]


def image_tag_only(ref):
    """'registry:5000/ns/oai-flexric:dev-4c5d6e7f@sha256:ab..' -> 'dev-4c5d6e7f'.

    Splits on the last path segment first, so a registry port is never mistaken for
    the tag separator. An untagged reference is returned unchanged.
    """
    if not ref:
        return None
    ref = ref.split('@', 1)[0]
    last_segment = ref.rsplit('/', 1)[-1]
    return last_segment.rsplit(':', 1)[-1] if ':' in last_segment else ref


def find_last(lines, pattern, group=1):
    """Last regex match in the file wins -- later output supersedes earlier retries."""
    found = None
    regex = re.compile(pattern)
    for line in lines:
        match = regex.search(line)
        if match is not None:
            found = match.group(group).strip()
    return found


def find_all(lines, pattern, group=1):
    """Unique matches, in first-seen order."""
    seen = []
    regex = re.compile(pattern)
    for line in lines:
        for match in regex.finditer(line):
            value = match.group(group).strip()
            if value not in seen:
                seen.append(value)
    return seen


def parse_arch_sizes(lines):
    """Per-architecture sizes from the pipeline's IMAGE SIZE markers."""
    sizes = {}
    for line in lines:
        match = ARCH_SIZE_RE.search(line)
        if match is not None:
            sizes[match.group(1)] = normalize_size(match.group(2))
    return sizes


def error_excerpt(lines, patterns, max_lines=12):
    """Pull the authoritative error out of a failed build log.

    Falls back to the log tail, because an infrastructure failure (killed agent,
    lost connection) leaves no recognisable error line at all.
    """
    for pattern in patterns:
        hits = [line.strip() for line in lines if re.search(pattern, line)]
        if hits:
            return hits[-max_lines:]
    tail = [line.rstrip() for line in lines if line.strip()][-max_lines:]
    return tail or ['Log file is empty -- the build produced no output.']


def empty_image(name, anchor):
    return {
        'name': name,
        'anchor': anchor,
        'status': None,
        'tag': None,
        # Metadata rows, as (label, value) pairs -- the Ubuntu and EL builds do not
        # expose the same properties, so each parser decides which rows it shows.
        'meta': [],
        'sizes': {},
        'size': None,
        'error': [],
    }


def parse_ubuntu(lines, args):
    """Parse archives/flexric_ubuntu_image_build.log (docker buildx, --progress=plain)."""
    image = empty_image('Build Ubuntu Image', 'build-flexric-ubuntu')
    if lines is None:
        image['meta'] = [('Dockerfile', args.ubuntu_dockerfile), ('Builder', None),
                         ('Platforms', None), ('Base Image', None)]
        image['error'] = [f'Log file ({UBUNTU_LOG}) not found -- Please check the run.']
        return image

    image['status'] = find_last(lines, r'OAI-FLEXRIC UBUNTU IMAGE BUILD:\s*(OK|KO)')

    # Image reference. A multi-arch push logs some of these without a tag.
    for pattern in (r'pushing manifest for\s+(\S+?)(?:@sha256:[0-9a-f]+)?(?:\s|$)',
                    r'merging manifest list\s+(\S+)',
                    r'naming to\s+(\S+?)\s*(?:done)?\s*$',
                    r'--tag[= ]+(\S+)'):
        tagged = [ref for ref in find_all(lines, pattern) if has_tag(ref)]
        if tagged:
            image['tag'] = tagged[-1]
            break

    # BuildKit logs only the Dockerfile's basename; the pipeline knows the full path.
    dockerfile = find_last(lines, r'load build definition from\s+(\S+)')
    if dockerfile and '/' not in dockerfile and args.ubuntu_dockerfile.endswith(dockerfile):
        dockerfile = args.ubuntu_dockerfile

    # BuildKit names its builder instance and driver on the very first line.
    builder = find_last(lines, r'building with "([^"]+)" instance using \S+ driver')
    driver = find_last(lines, r'building with "[^"]+" instance using (\S+) driver')
    if builder and driver:
        builder = f'{builder} ({driver} driver)'

    platforms = find_all(lines, r'\[(linux/[a-z0-9]+)[\s\]]')
    bases = [short_image_ref(ref)
             for ref in find_all(lines, r'load metadata for\s+(\S+)')]

    image['meta'] = [
        ('Dockerfile', dockerfile or args.ubuntu_dockerfile),
        ('Builder', builder),
        ('Platforms', ', '.join(platforms) if platforms else None),
        ('Base Image', ', '.join(bases) if bases else None),
    ]

    image['sizes'] = parse_arch_sizes(lines)
    # "docker image ls" output appended by the pipeline; absent for a --push build,
    # which never writes the image into the local store.
    size = find_last(lines, r'^oai-flexric\s+\S+\s+\w+\s+.*?\s+([0-9.]+\s*[KMGT]?B)\s*$')
    if size:
        image['size'] = normalize_size(size)

    if image['status'] != 'OK':
        image['error'] = error_excerpt(lines, [
            r'^ERROR: failed to solve:',
            r'^ERROR:',
            r'^error:',
        ])
    return image


def parse_el(lines, args):
    """Parse archives/flexric_el_image_build.log (OpenShift/podman build).

    The OpenShift build is single-platform and driven by a BuildConfig rather than a
    named builder instance, so it shows neither a Builder nor a Platforms row.
    """
    image = empty_image('Build Enterprise Linux Image', 'build-flexric-el')
    if lines is None:
        image['meta'] = [('Dockerfile', args.el_dockerfile), ('Base Image', None)]
        image['error'] = [f'Log file ({EL_LOG}) not found -- Please check the run.']
        return image

    image['status'] = find_last(lines, r'OAI-FLEXRIC EL IMAGE BUILD:\s*(OK|KO)')

    for pattern in (r'Pushing image\s+(\S+)',
                    r'Successfully pushed\s+(\S+)',
                    r'COMMIT\s+(\S+)',
                    r'Successfully tagged\s+(\S+)'):
        tagged = [ref for ref in find_all(lines, pattern) if has_tag(ref)]
        if tagged:
            image['tag'] = tagged[-1]
            break

    base = find_last(lines, r'STEP 1/[0-9]+:\s*FROM\s+(\S+)')
    image['meta'] = [
        ('Dockerfile', args.el_dockerfile),
        ('Base Image', base),
    ]

    image['sizes'] = parse_arch_sizes(lines)
    # Appended by "oc describe istag ... | grep 'Image Size:'". It reports the
    # compressed registry size, so label it as such rather than implying on-disk size.
    size = find_last(lines, r'Image Size:\s*([0-9.]+\s*[KMGT]?B)')
    if size:
        image['size'] = f'{normalize_size(size)} (compressed)'

    if image['status'] != 'OK':
        image['error'] = error_excerpt(lines, [
            r'^error: build error:',
            r'^error:',
            r'^Error:',
            r'^ERROR:',
        ])
    return image


def parse_warnings(lines):
    """Compiler diagnostics from the Ubuntu build log.

    Deduplicated: a multi-arch build compiles every file once per platform, so each
    warning otherwise appears two or more times with nothing to distinguish the rows.
    """
    seen = []
    if lines is None:
        return seen
    for line in lines:
        match = WARNING_RE.match(line.strip())
        if match is None:
            continue
        entry = (match.group(1), match.group(2), match.group(3), match.group(4).strip())
        if entry not in seen:
            seen.append(entry)
    return seen


def parse_ctest(lines):
    """One CTest run: the per-test outcomes plus whether the whole run passed."""
    result = {'found': lines is not None, 'tests': [], 'failed': 0, 'total': 0,
              'error': None, 'build_failed': False}
    if lines is None:
        return result

    in_section = False
    for line in lines:
        if 'Test project /flexric/build' in line:
            in_section = True
        if in_section and 'Total Test time' in line:
            in_section = False
        if not in_section:
            continue
        match = CTEST_RESULT_RE.search(line)
        if match is not None:
            result['tests'].append((match.group(1), match.group(2) == 'Passed'))

    summary = None
    for line in lines:
        found = CTEST_SUMMARY_RE.search(line)
        if found is not None:
            summary = found
    if summary is not None:
        result['failed'] = int(summary.group(2))
        result['total'] = int(summary.group(3))
    else:
        result['total'] = len(result['tests'])
        result['failed'] = sum(1 for _, ok in result['tests'] if not ok)

    if not result['tests']:
        result['error'] = error_excerpt(lines, [
            r'^#\d+ \S+ (?:fatal|error):',
            r'^#\d+ ERROR:',
            r'^ERROR: failed to solve:',
        ])
        result['build_failed'] = any(
            re.search(r'^(?:#\d+ )?ERROR:', line) for line in lines)
    return result


# ---------------------------------------------------------------------------
# HTML rendering
# ---------------------------------------------------------------------------

GLYPH_OK = '<span class="glyphicon glyphicon-ok" style="color:#5cb85c"></span>'
GLYPH_KO = '<span class="glyphicon glyphicon-remove" style="color:#d9534f"></span>'


def esc(value):
    return html.escape(str(value)) if value is not None else ''


def git_row(glyph, label, value):
    """One row of the git information table, in the ci-scripts/common layout."""
    shown = value if value not in (None, '') else MISSING
    return (f'    <tr>\n'
            f'      <td bgcolor="lightcyan"> <span class="glyphicon glyphicon-{glyph}">'
            f'</span> {esc(label)}</td>\n'
            f'      <td>{shown}</td>\n'
            f'    </tr>')


def render_git_info(args):
    url = esc(args.git_url)
    rows = [git_row('wrench', 'Build Trigger',
                    'Pull Request' if args.pull_request else 'Push Event'),
            git_row('cloud-upload', 'GIT Repository', f'<a href="{url}">{url}</a>')]

    if args.pull_request:
        pr_url = esc(args.pr_url)
        rows.append(git_row('log-out', 'Pull Request URL',
                            f'<a href="{pr_url}">{pr_url}</a>' if pr_url else ''))
        rows.append(git_row('header', 'Pull Request Title', esc(args.pr_title)))
        rows.append(git_row('log-out', 'Source Branch', esc(args.git_src_branch)))
        rows.append(git_row('tag', 'Source Commit ID', esc(args.git_src_commit)))
        rows.append(git_row('log-in', 'Target Branch', esc(args.git_dst_branch)))
        rows.append(git_row('tag', 'Target Commit ID', esc(args.git_dst_commit)))
    else:
        rows.append(git_row('tree-deciduous', 'Branch', esc(args.git_src_branch)))
        rows.append(git_row('tag', 'Commit ID', esc(args.git_src_commit)))

    body = '\n'.join(rows)
    return (f'  <table class="table-bordered" width="80%" align="center" border="1">\n'
            f'{body}\n'
            f'  </table>\n  <br>')


def meta_row(label, value):
    shown = f'<code>{esc(value)}</code>' if value not in (None, '', []) else MISSING
    return (f'<tr><td bgcolor="lightcyan" style="width:22%;white-space:nowrap">'
            f'{esc(label)}</td><td>{shown}</td></tr>')


def render_info_cell(image):
    """Target image size when the build passed, the extracted error when it failed."""
    if image['status'] != 'OK':
        body = '\n'.join(esc(line) for line in image['error'])
        return ('<pre style="border:none;background-color:#fff5f5;margin-top:6px;'
                f'white-space:pre-wrap;word-break:break-word">{body}</pre>')

    if image['sizes']:
        rows = '<br>'.join(f'<code>{esc(platform)}: {esc(size)}</code>'
                           for platform, size in sorted(image['sizes'].items()))
        return f'Target image size is:<br>{rows}'

    if image['size']:
        return f'Target image size is <code>{esc(image["size"])}</code>'

    return ('Image built, size not reported &mdash; append an <code>IMAGE SIZE '
            '&lt;platform&gt;: &lt;size&gt;</code> line per architecture to report it')


def render_pills(images):
    """Bootstrap nav-pills, one per image, the first one active."""
    items = []
    for index, image in enumerate(images):
        glyph = GLYPH_OK if image['status'] == 'OK' else GLYPH_KO
        active = ' class="active"' if index == 0 else ' class=""'
        items.append(f'<li{active}><a data-toggle="pill" href="#{image["anchor"]}">'
                     f'{glyph} {esc(image["name"])}</a></li>')
    return f'<ul class="nav nav-pills">{"".join(items)}</ul>'


def render_image_pane(image, index):
    status = image['status']
    if status == 'OK':
        status_cell = '<td bgcolor="lightgreen"><b>OK</b></td>'
    else:
        label = 'KO' if status == 'KO' else 'NOT RUN'
        status_cell = f'<td bgcolor="lightcoral"><b>{label}</b></td>'

    final_label = 'PASS' if status == 'OK' else 'FAIL'
    final_color = 'green' if status == 'OK' else 'red'

    tag = (f'<code>{esc(image_tag_only(image["tag"]))}</code>'
           if image['tag'] else MISSING)
    meta = '\n'.join(meta_row(label, value) for label, value in image['meta'])
    pane_class = 'tab-pane fade in active' if index == 0 else 'tab-pane fade'

    return f"""<div id="{image['anchor']}" class="{pane_class}">
<h3>{esc(image['name'])}</h3>
<table class="table-bordered" width="100%" border="1">
{meta}
</table>
<br>
<table class="table-bordered" width="100%" border="1">
  <tr bgcolor="#33CCFF">
    <th style="width:22%">Image Tag</th>
    <th style="width:10%">Status</th>
    <th>Info</th>
  </tr>
  <tr>
    <td>{tag}</td>
    {status_cell}
    <td>{render_info_cell(image)}</td>
  </tr>
  <tr>
    <td colspan="3" style="border:none;background:transparent;height:14px;padding:0"></td>
  </tr>
  <tr>
    <th bgcolor="#33CCFF" colspan="2">Final Build Status</th>
    <th bgcolor="{final_color}"><font color="white">{final_label}</font></th>
  </tr>
</table>
</div>"""


def render_build_summary(images):
    failed = [img['name'] for img in images if img['status'] != 'OK']
    if failed:
        alert = ('<div class="alert alert-danger"><strong>One or more container images '
                 'were not created</strong></div>')
    else:
        alert = ('<div class="alert alert-success"><strong>All Container Target Images '
                 'were created.</strong></div>')

    panes = ''.join(render_image_pane(img, i) for i, img in enumerate(images))
    return (f'<h2>Container Images Build Summary</h2>{alert}'
            f'{render_pills(images)}'
            f'<div class="tab-content">{panes}</div>\n  <br>')


def render_compilation_warnings(warnings):
    """H3 + collapsible table of compiler diagnostics, matching the existing report.

    No alert banner here: the existing FlexRIC report puts the H3 straight above the
    collapse button, so an empty-warnings run gets a single explanatory row inside the
    table rather than a second markup vocabulary.
    """
    if warnings:
        rows = []
        for path, line_no, kind, message in warnings:
            colour = 'Orange' if kind == 'warning' else 'Tomato'
            rows.append(f'    <tr>\n'
                        f'      <td>{esc(path)}</td>\n'
                        f'      <td>{esc(line_no)}</td>\n'
                        f'      <td bgcolor="{colour}">{esc(kind)}</td>\n'
                        f'      <td>{esc(message)}</td>\n'
                        f'    </tr>')
        body = '\n'.join(rows)
    else:
        body = ('    <tr>\n'
                '      <td colspan="4">No compilation errors or warnings were found.</td>\n'
                '    </tr>')

    return f"""<h2>Compilation Warnings Details</h2>
  <br>
  <button data-toggle="collapse" data-target="#oai-compilation-details">Details for Compilation Errors and Warnings</button>
  <br>
  <div id="oai-compilation-details" class="collapse">
  <br>
  <table class="table-bordered" width="90%" align="center" border="1">
    <tr bgcolor="#33CCFF">
      <th>File Name</th>
      <th>Line Number</th>
      <th>Status</th>
      <th>Error / Warning Message</th>
    </tr>
{body}
  </table>
  <br>
  </div>
  <br>"""


def render_ctest_chapter(title, result):
    """One CTest chapter: H2, pass/fail alert, then the per-test list."""
    if not result['found']:
        return (f'<h2>{esc(title)}</h2>\n'
                '<div class="alert alert-danger">\n'
                '  <strong>CTests report file not found! Please check the run.</strong>\n'
                '</div>')

    if not result['tests']:
        message = ('CTests did not run: the image build failed'
                   if result['build_failed'] else 'No CTest results found in the log')
        level = 'danger'
    elif result['failed']:
        message = f"{result['failed']} of {result['total']} CTests failed"
        level = 'danger'
    else:
        message, level = 'All CTests passed', 'success'

    items = '\n'.join(
        f'    <li class="list-group-item list-group-item-{"success" if ok else "danger"}">'
        f'<i class="glyphicon glyphicon-{"ok" if ok else "remove"}"></i>{esc(name)}</li>'
        for name, ok in result['tests'])
    listing = f'\n  <br>\n  <ul class="list-group">\n{items}\n  </ul>' if items else ''
    if not items and result['error']:
        body = '\n'.join(esc(line) for line in result['error'])
        listing = ('\n  <pre style="border:none;background-color:#fff5f5;margin-top:6px;'
                   f'white-space:pre-wrap;word-break:break-word">{body}</pre>')

    return (f'<h2>{esc(title)}</h2>\n'
            f'<div class="alert alert-{level}">\n'
            f'  <strong>{esc(message)}</strong>\n'
            f'</div>{listing}')


def render_ctests(ctests):
    return '\n'.join(render_ctest_chapter(title, result) for title, result in ctests)


def render_report(args, images, warnings, ctests):
    """Fill the page skeleton in report-template.html.

    string.Template ($name) rather than str.format, because the inline CSS in the
    template is full of braces that .format would try to interpret. substitute()
    raises on an unknown placeholder, so a typo fails loudly instead of shipping a
    literal "$foo" into the report.
    """
    with open(args.template, 'r', encoding='utf-8') as handle:
        skeleton = string.Template(handle.read())
    build_link = (f'<a href="{esc(args.build_url)}">{esc(args.build_id)}</a>'
                  if args.build_url else esc(args.build_id))
    return skeleton.substitute(
        job_name=esc(args.job_name),
        build_id=esc(args.build_id),
        build_link=build_link,
        git_info=render_git_info(args),
        build_summary=render_build_summary(images),
        compilation_warnings=render_compilation_warnings(warnings),
        ctests=render_ctests(ctests),
        year=datetime.date.today().strftime('%Y'),
    )


def parse_args():
    parser = argparse.ArgumentParser(
        description='Generate the FlexRIC CI HTML report.')
    parser.add_argument('--job-name', default='FlexRIC', help='Jenkins job name')
    parser.add_argument('--build-id', default='N/A', help='Jenkins build number')
    parser.add_argument('--build-url', default='', help='Jenkins build URL')
    parser.add_argument('--git-url', default='', help='Git repository URL')
    parser.add_argument('--git-src-branch', default='', help='Source branch')
    parser.add_argument('--git-src-commit', default='', help='Source commit SHA')
    parser.add_argument('--pull-request', action='store_true',
                        help='Render the pull-request flavour of the git info table')
    parser.add_argument('--pr-number', default='', help='Pull request number')
    parser.add_argument('--pr-url', default='', help='Pull request URL')
    parser.add_argument('--pr-title', default='', help='Pull request title')
    parser.add_argument('--git-dst-branch', default='', help='Target branch (PR only)')
    parser.add_argument('--git-dst-commit', default='', help='Target commit (PR only)')
    # BuildKit logs only a basename, and the OpenShift log names no Dockerfile at all.
    parser.add_argument('--ubuntu-dockerfile', default='docker/Dockerfile.flexric.ubuntu',
                        help='Dockerfile used for the Ubuntu image')
    parser.add_argument('--el-dockerfile', default='docker/Dockerfile.flexric.centos',
                        help='Dockerfile used for the Enterprise Linux image')
    parser.add_argument('--template', default=TEMPLATE_PATH,
                        help='Page skeleton to fill (default: report-template.html '
                             'next to this script)')
    parser.add_argument('--archives', default='archives',
                        help='Directory holding the build logs (default: archives)')
    parser.add_argument('--output', default=REPORT_NAME,
                        help=f'Report file to write (default: {REPORT_NAME})')
    args, unknown = parser.parse_known_args()
    if unknown:
        print(f'Ignoring unknown arguments: {" ".join(unknown)}', file=sys.stderr)
    return args


def main():
    args = parse_args()
    try:
        ubuntu_lines = read_log(os.path.join(args.archives, UBUNTU_LOG))
        images = [
            parse_ubuntu(ubuntu_lines, args),
            parse_el(read_log(os.path.join(args.archives, EL_LOG)), args),
        ]
        warnings = parse_warnings(ubuntu_lines)
        ctests = [(title, parse_ctest(read_log(os.path.join(args.archives, log))))
                  for log, title in CTEST_LOGS]
        with open(args.output, 'w', encoding='utf-8') as report:
            report.write(render_report(args, images, warnings, ctests))
    except Exception:
        # Exit 2, distinct from the exit 1 below: the caller must be able to tell a
        # broken generator (missing template, bad placeholder) from a failed build.
        traceback.print_exc()
        return 2

    for image in images:
        print(f'{image["name"]}: {image["status"] or "NOT RUN"}')
    print(f'Compilation diagnostics: {len(warnings)} distinct')
    for title, result in ctests:
        if not result['found']:
            print(f'{title}: NOT RUN')
        else:
            print(f'{title}: {result["total"] - result["failed"]}/{result["total"]} passed')
    print(f'Report written to {args.output}')

    # Exit 1 lets the pipeline judge the build from the report instead of re-parsing logs.
    return 0 if all(img['status'] == 'OK' for img in images) else 1


if __name__ == '__main__':
    sys.exit(main())
