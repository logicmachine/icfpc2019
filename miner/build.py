import os
import sys
import subprocess
import yaml


def main():
    yaml_file = sys.argv[1]
    root = os.path.dirname(yaml_file)
    with open(yaml_file, 'r') as f:
        config = yaml.load(f, Loader=yaml.FullLoader)
        for name in config:
            print('------------------------------')
            print(' ', name)
            print('------------------------------')
            cwd = os.path.join(root, name)
            proc = subprocess.Popen(['./build.sh'], cwd=cwd) 
            ret = proc.wait()
            if ret == 0:
                print('Success!!')
            else:
                print('Failed with return code {}'.format(ret))
            print('')


if __name__ == '__main__':
    main()
