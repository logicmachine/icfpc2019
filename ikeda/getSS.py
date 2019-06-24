import time
import os.path

from selenium import webdriver
from selenium.webdriver.chrome.options import Options

options = Options()
options.binary_location = '/usr/bin/google-chrome'
options.add_argument('--headless')
options.add_argument('--window-size=1200,1600')

driver = webdriver.Chrome('chromedriver', chrome_options=options)
driver.get('https://icfpcontest2019.github.io/solution_visualiser/')

# visualizer DOM?
element = driver.find_element_by_id('submit_task') 

input_data_path = './ss'
files = []
for filename in os.listdir(input_data_path):
    print(filename)
    element.send_keys(os.path.abspath(input_data_path) + '/' + filename)
    time.sleep(5)
    driver.save_screenshot('./screenshot/' + filename + '.png')

driver.quit()
