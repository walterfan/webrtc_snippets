#!/usr/bin/env python3

from PIL import Image
import pytesseract
import cv2
import re
import os
import sys



URL_PATTERN = r"(?i)\b((?:https?://|www\d{0,3}[.]|[a-z0-9.\-]+[.][a-z]{2,4}/)(?:[^\s()<>]+|\(([^\s()<>]+|(\([^\s()<>]+\)))*\))+(?:\(([^\s()<>]+|(\([^\s()<>]+\)))*\)|[^\s`!()\[\]{};:'\".,<>?«»“”‘’]))"

def extract_urls(image_file):
    img=cv2.imread(image_file)

    gray=cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)

    gray_image="gray_{}_{}".format(os.getpid(), image_file)

    cv2.imwrite(gray_image, gray)
    print("begin to extract urls from {}".format(gray_image))
    text=pytesseract.image_to_string(Image.open(gray_image))
    os.remove(gray_image)

    print("ocr result: ", text)

    urls = re.findall(URL_PATTERN, text)
    links = []
    for url in urls:
        link = '<a href="{}" target="_blank">{}</a>'.format(url[0], url[0])
        print(link)
        links.append(link)
    return links

if __name__ == '__main__':
    image_file  = "test.png"
    if len(sys.argv) > 1:
        image_file = sys.argv[1]

    extract_urls(image_file)