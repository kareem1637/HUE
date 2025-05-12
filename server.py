from flask import Flask, render_template, jsonify, request, send_file, Response
import numpy as np
import cv2
import requests
import os
from threading import Lock
from werkzeug.utils import secure_filename
app = Flask(__name__)
BASE_DIR = os.getcwd()
RESULT_FOLDER = os.path.join(BASE_DIR, 'static/results')
os.makedirs(RESULT_FOLDER, exist_ok=True)
app.config['UPLOAD_FOLDER'] = RESULT_FOLDER
ESP32_CAM_URL = 'http://192.168.4.1'

# Global variables for result sharing
latest_result = None
result_lock = Lock()
def get_frame():
    resp = requests.get(f'{ESP32_CAM_URL}/stream', stream=True)
    return Response(resp.iter_content(chunk_size=1024), content_type='multipart/x-mixed-replace; boundary=frame')

@app.route('/')
def index():
    return render_template('results.html')
@app.route('/video_feed')
def video_feed():
    return get_frame()
def analyze_image(img, folder_path,filename):
    # Image processing
    hsv_roi = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    # Define a hue range for yellow (e.g., 15 to 45)
    yellow_lower = np.array([15, 100, 50])  # Hue range, low saturation, and brightness
    yellow_upper = np.array([40, 255, 255])  # Hue range, high saturation, and brightness

    mask_yellow = cv2.inRange(hsv_roi, yellow_lower, yellow_upper)

    # Morphological operations
    kernel = np.ones((5, 5), np.uint8)
    mask_cleaned = cv2.morphologyEx(mask_yellow, cv2.MORPH_CLOSE, kernel)
    # Apply the mask to the image to extract the yellow pixels
    yellow_pixels = cv2.bitwise_and(img, img, mask=mask_cleaned)

    # Calculate the mean for Hue, Saturation, and Value in HSV
    mean_hue = cv2.mean(hsv_roi[:, :, 0], mask=mask_cleaned)[0]
    mean_saturation = cv2.mean(hsv_roi[:, :, 1], mask=mask_cleaned)[0]
    mean_value = cv2.mean(hsv_roi[:, :, 2], mask=mask_cleaned)[0]
    # Save mask image
    mask_path = os.path.join(folder_path,f"{filename}_mask.jpeg")
    cv2.imwrite(mask_path, yellow_pixels)


  


    return  mask_path,  mean_hue,mean_saturation,mean_value

@app.route('/trigger-capture', methods=['POST'])
def trigger_capture():
    try:
        # Just trigger the capture and return immediately
        # Fetch the image as raw binary data
        filename = request.form.get('filename')
        if not filename:
            return jsonify({"error": "Filename is required"}), 400
        response = requests.get(f'{ESP32_CAM_URL}/capture', timeout=10, stream=True)
        response.raise_for_status()

        # Read the image data from the response
        img_data = response.content  # Raw binary image data
        # Secure the filename and save the image
        filename = secure_filename(filename) 
        folder_path = os.path.join(app.config['UPLOAD_FOLDER'], filename)
        os.makedirs(folder_path, exist_ok=True)  # Create folder if not exists
        image_path = os.path.join(folder_path, filename+".jpeg")
        with open(image_path, "wb") as f:
            f.write(img_data)   

        # Analyze the image
        img_np = cv2.imread(image_path)
        if img_np is None:
            return "Failed to decode image", 400

        mask_path, mean_hue,mean_saturation,mean_value = analyze_image(img_np, folder_path,filename)
        # Store the result
        with result_lock:
            global latest_result
            latest_result = {
                "image_filename": image_path,
                "mask_filename": mask_path,
                'mean_saturation': mean_saturation,
                'mean_value': mean_value,
                'mean_hue':mean_hue
            }

        return jsonify({"status": "Image captured and processed"}), 200
    except requests.exceptions.RequestException as e:
        print("Error contacting ESP32-CAM:", e)
        return jsonify({"error": str(e)}), 500

@app.route('/check-result', methods=['GET'])
def check_result():
    global latest_result
    with result_lock:
        if latest_result:
            result = latest_result
            latest_result = None  # Clear after sending
            return jsonify(result), 200
        return jsonify({"status": "processing"}), 202



@app.route('/results/<path:image_path>')
def get_image(image_path):
    return send_file(image_path, mimetype='image/jpeg')

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=5000, threaded=True)