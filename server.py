from flask import Flask, request, jsonify, render_template_string
import requests

app = Flask(__name__)

# Variables globales para almacenar las IPs
last_ip_cam = None      # IP recibida de la ESP32-CAM
last_ip_motor = None    # IP recibida de otra ESP (ej: control de motores)
camera_response = None  # Respuesta de la ESP-CAM

@app.route('/set_ip', methods=['GET'])
def set_ip():
    global last_ip_cam
    ip_value = request.args.get('set_ip')
    
    if ip_value:
        last_ip_cam = ip_value
        return jsonify({"status": "success", "ip": ip_value}), 200
    else:
        return jsonify({"status": "error", "message": "No IP provided"}), 400

@app.route('/get_ip', methods=['GET'])
def get_ip():
    if last_ip_cam:
        return jsonify({"status": "success", "last_ip": last_ip_cam}), 200
    else:
        return jsonify({"status": "error", "message": "No IP set yet"}), 404

@app.route('/set_ip_motor', methods=['GET'])
def set_ip_motor():
    global last_ip_motor
    ip_value = request.args.get('set_ip_motor')
    
    if ip_value:
        last_ip_motor = ip_value
        return jsonify({"status": "success", "ip": ip_value}), 200
    else:
        return jsonify({"status": "error", "message": "No IP provided"}), 400

@app.route('/get_ip_motor', methods=['GET'])
def get_ip_motor():
    if last_ip_motor:
        return jsonify({"status": "success", "last_ip_motor": last_ip_motor}), 200
    else:
        return jsonify({"status": "error", "message": "No motor IP set yet"}), 404

@app.route('/', methods=['GET'])
def home():
    html = """
    <html>
    <head><title>Servidor Flask - ESPCam y Motor</title></head>
    <body>
        <h1>Servidor de Registro de IPs</h1>
        <p><strong>Última IP de la ESPCam:</strong> {{ last_ip_cam if last_ip_cam else "No registrada aún." }}</p>
        <p><strong>Última IP de la esp:</strong> {{ last_ip_motor if last_ip_motor else "No registrada aún." }}</p>
    </body>
    </html>
    """
    return render_template_string(html, last_ip_cam=last_ip_cam, last_ip_motor=last_ip_motor)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
