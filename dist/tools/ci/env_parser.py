#! /usr/bin/env python3
"""Gets the metadata information for the test environment.

This includes things like python modules, environment variables, toolchains
and repo information.
The data can be presented in a console for developers or xml format to feed
into test artifacts.
"""
import argparse
import logging
import os
import subprocess
import re
import xml.etree.ElementTree as ET
try:
    import git
except ImportError:
    git = None
import pkg_resources


LOG_HANDLER = logging.StreamHandler()
LOG_HANDLER.setFormatter(logging.Formatter(logging.BASIC_FORMAT))

LOG_LEVELS = ('debug', 'info', 'warning', 'error', 'fatal', 'critical')


def python_modules_to_dict(flatten=False):
    py_modules = ['riot_pal', 'philip_pal', 'robotframework', 'pyserial',
                  'wiringpi', 'smbus', 'deepdiff']
    py_versions = {}
    for pm in py_modules:
        try:
            py_versions[pm] = pkg_resources.get_distribution(pm).version
        except pkg_resources.DistributionNotFound as exc:
            logging.debug("%r", exc)
            py_versions[pm] = 'missing'
    ret = {}
    if flatten:
        ret.update(py_versions)
    else:
        ret["python_modules"] = py_versions
    return ret


def env_to_dict(flatten=False):
    specific_keys = ["PORT", "PATH"]
    accepted_keys = ["DEBUG", "PHILIP", "BPH", "BOARD", "HIL", "RIOT",
                     "DUT", "RF_", "RESET", "ESP", "WAIT", "JLINK", "PERIPH",
                     "OPENOCD", "FLASH", "PYTHON"]
    safe_env_vars = {}
    for key in specific_keys:
        if os.environ.get(key) is not None:
            safe_env_vars[key] = os.environ.get(key)
    for key in os.environ.keys():
        if any(accepted_key in key for accepted_key in accepted_keys):
            if os.environ[key] is not None:
                safe_env_vars[key] = os.environ[key]
    ret = {}
    if flatten:
        ret.update(safe_env_vars)
    else:
        ret["environment_variables"] = safe_env_vars
    return ret


def toolchain_versions_to_dict(print_toolchain_versions_path, flatten=False):
    result = subprocess.run([print_toolchain_versions_path],
                            stdout=subprocess.PIPE)
    result = result.stdout.decode('utf-8')

    sep_headers_pattern = re.compile(r'(.*)\n-+\n((?:.|\n[^\n])*)',
                                     re.MULTILINE)
    riot_env = {}
    for (header, content) in re.findall(sep_headers_pattern, result):
        content_to_dict = re.compile(r'^\s*(.*?): (.*)', re.MULTILINE)
        meta_params = {}
        for (key, val) in re.findall(content_to_dict, content):
            meta_params[key.replace(' ', '_').replace('\'', '').lower()] = val
        if flatten:
            riot_env.update(meta_params)
        else:
            riot_env[header.replace(' ', '_').lower()] = meta_params
    return riot_env


def get_repo_val(repo_path, repo_name=None, flatten=False):
    logging.debug("Getting repo info from %r", repo_path)
    try:
        repo = git.Repo(repo_path)
    except git.exc.InvalidGitRepositoryError as exc:
        logging.info("%r", exc)
        return []
    remote_url = repo.remotes[0].config_reader.get("url")
    if repo_name is None:
        repo_name = os.path.splitext(os.path.basename(remote_url))[0]
        repo_name = repo_name.lower().replace('-', '_')
    prefix_name = ""
    if flatten:
        prefix_name = repo_name + "_"
    repo_params = {}

    try:
        commit = repo.head.commit
        repo_params['{}commit_id'.format(prefix_name)] = commit.hexsha
    except TypeError as exc:
        logging.warning("commit_id: %r", exc)
    try:
        commit = repo.head.commit
        td = commit.authored_datetime.strftime("%Y-%m-%dT%H:%M:%S")
        repo_params['{}commit_timestamp'.format(prefix_name)] = td
    except TypeError as exc:
        logging.warning("commit_timestamp: %r", exc)
    try:
        branch = repo.active_branch
        repo_params['{}branch_name'.format(prefix_name)] = branch.name
    except TypeError as exc:
        logging.debug("branch_name: %r", exc)
    try:
        repo_params['{}version'.format(prefix_name)] = repo.git.describe()
    except git.exc.GitCommandError:
        logging.debug("%r has no version", repo_name)
    ret = {}
    if not flatten:
        ret[repo_name] = repo_params
    else:
        ret.update(repo_params)
    return ret


def print_env_to_console(env, flatten=False):
    largest_str = 0
    for envk, envv in env.items():
        if flatten:
            largest_str = max(largest_str, len(envk))
        else:
            for nk, nv in envv.items():
                largest_str = max(largest_str, len(nk))
    for envk, envv in env.items():
        if flatten:
            print("{: >{l}}: {}".format(envk, envv, l=largest_str))
        else:
            print(envk.title().replace('_', ' '))
            print('='*80)
            for nk, nv in envv.items():
                print("{: >{l}}: {}".format(nk, nv, l=largest_str))
            print("")


def dict_to_etree(d):
    def _to_etree(d, root):
        if not d:
            pass
        elif isinstance(d, str):
            root.text = d
        elif isinstance(d, dict):
            for k, v in d.items():
                assert isinstance(k, str)
                if k.startswith('#'):
                    assert k == '#text' and isinstance(v, str)
                    root.text = v
                elif k.startswith('@'):
                    assert isinstance(v, str)
                    root.set(k[1:], v)
                elif isinstance(v, list):
                    for e in v:
                        _to_etree(e, ET.SubElement(root, k))
                else:
                    _to_etree(v, ET.SubElement(root, k))
        else:
            raise TypeError('invalid type: ' + str(type(d)))
    assert isinstance(d, dict) and len(d) == 1
    tag, body = next(iter(d.items()))
    node = ET.Element(tag)
    _to_etree(body, node)
    return node


_PATH = os.path.dirname(os.path.abspath(__file__))
_RF_DIR = os.path.join(_PATH, '../../../')
_RIOT_DIR = os.path.join(_RF_DIR, 'RIOT')

PARSER = argparse.ArgumentParser()
PARSER.add_argument('--riot-dir', default=_RIOT_DIR,
                    help='Directory of RIOT repo')
PARSER.add_argument('--rf-dir', default=_RF_DIR,
                    help='Directory of RobotFW-Tests repo')
PARSER.add_argument('--output', default="meta.xml",
                    help='file path of the result xml')
PARSER.add_argument('--flatten', '-f', default=False, action='store_true',
                    help='Flattens output, gets rid of nesting')
PARSER.add_argument('--console', '-c', default=False, action='store_true',
                    help='Prints to console')
PARSER.add_argument('--xml', '-x', default=False, action='store_true',
                    help='Writes to an xml file')
PARSER.add_argument('--git', '-g', default=False, action='store_true',
                    help='Writes git related output')
PARSER.add_argument('--env', '-e', default=False, action='store_true',
                    help='Writes environment related output')
PARSER.add_argument('--py', '-p', default=False, action='store_true',
                    help='Writes python module related output')
PARSER.add_argument('--tool', '-t', default=False, action='store_true',
                    help='Writes toolchain related output')
PARSER.add_argument('--loglevel', choices=LOG_LEVELS, default='info',
                    help='Python logger log level')


def main(args):
    """Create a xml or print containing information on the test environment."""
    if args.loglevel:
        loglevel = logging.getLevelName(args.loglevel.upper())
        logging.basicConfig(level=loglevel)

    env = {}
    if args.git:
        if git is None:
            logging.info("Cannot collect repo information")
            logging.info("git module missing")
            logging.info("try to pip install gitpython")
        else:
            env.update(get_repo_val(args.riot_dir, repo_name="RIOT",
                                    flatten=args.flatten))
            env.update(get_repo_val(args.rf_dir, repo_name="RobotFW-Tests",
                                    flatten=args.flatten))
    if args.env:
        env.update(env_to_dict(flatten=args.flatten))
    if args.py:
        env.update(python_modules_to_dict(flatten=args.flatten))
    if args.tool:
        ptv_path = 'dist/tools/ci/print_toolchain_versions.sh'
        print_toolchain_versions_path = os.path.join(args.riot_dir, ptv_path)
        env.update(toolchain_versions_to_dict(print_toolchain_versions_path,
                                              flatten=args.flatten))

    if args.console:
        print_env_to_console(env, args.flatten)
    if args.xml:
        xml_et = ET.ElementTree(dict_to_etree({"metadata": env}))
        xml_et.write(args.output, encoding="UTF-8", xml_declaration=True)


if __name__ == '__main__':
    main(PARSER.parse_args())
