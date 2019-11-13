#! /usr/bin/env python3
import pkg_resources
import os

specific_keys = ["PORT", "PATH"]
accepted_keys = ["DEBUG", "PHILIP", "BPH", "BOARD", "HIL", "RIOT", 
                 "DUT", "RF_", "RESET", "ESP", "WAIT", "JLINK", "PERIPH",
                 "OPENOCD", "FLASH", "PYTHON"]

safe_env_vars = {}
for key in specific_keys:
    safe_env_vars[key] = os.environ.get(key)
for key in os.environ.keys():
    if any(accepted_key in key for accepted_key in accepted_keys):
        safe_env_vars[key] = os.environ[key]

header_name = "Environment Variables"
print(header_name)
print(len(header_name) * '-')       
for k, v in safe_env_vars.items():
    print("{: >23}: {}".format(k, v))
print("")


py_modules = ['riot_pal', 'philip_pal', 'robotframework', 'pyserial',
              'wiringpi', 'smbus', 'deepdiff']
py_versions = {}
for pm in py_modules:
    try:
        py_versions[pm] = pkg_resources.get_distribution(pm).version
    except pkg_resources.DistributionNotFound as exc:
        py_versions[pm] = 'None'

header_name = "Python Package Versions"
print(header_name)
print(len(header_name) * '-')
for k, v in py_versions.items():
    print("{: >23}: {}".format(k, v))
