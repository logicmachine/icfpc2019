import time
import os.path
import sys

from selenium import webdriver
from selenium.webdriver.chrome.options import Options

args = sys.argv
if len(args) != 3:
    print('invalid number of arguments.')
    sys.exit()

options = Options()
options.binary_location = '/usr/bin/google-chrome'
options.add_argument('--headless')
options.add_argument('--window-size=1200,1600')

driver = webdriver.Chrome('chromedriver', options=options)
driver.get('https://icfpcontest2019.github.io/solution_checker/')

# get DOM
task = driver.find_element_by_id('submit_task') 
solution = driver.find_element_by_id('submit_solution') 

task_path = './' + args[1]
solution_path = './' + args[2]

task.send_keys(os.path.abspath(task_path))
solution.send_keys(os.path.abspath(solution_path))

driver.find_element_by_id('execute_solution').click()
time.sleep(3)
status = driver.find_element_by_id('output').text

if status[0] == 'S':
    ret = status.split(' ')[4]
elif status[0] == 'F':
    ret = -1
elif status[0] == 'C':
    ret = -2

print(ret)

driver.quit()

