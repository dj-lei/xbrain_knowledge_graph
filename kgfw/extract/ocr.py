import easyocr
import cv2


class Ocr(object):
    def __init__(self):
        self.reader = easyocr.Reader(['en'])

    def preprocess(self, image_path):
        image = cv2.imread(image_path)
        image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        (origH, origW) = image.shape[:2]
        (newW, newH) = (int(origW * 1.5), int(origH * 1.5))
        image = cv2.resize(image, (newW, newH))
        return image

    def extract(self, image_path):
        image = self.preprocess(image_path)
        result = self.reader.readtext(image)
        return result
