from flask import Flask, request, jsonify, render_template_string
from datetime import datetime

app = Flask(__name__)

# Variables globales
last_ip_cam = None
last_ip_cam_time = None

last_ip_motor = None
last_ip_motor_time = None

@app.route('/set_ip', methods=['GET'])
def set_ip():
    global last_ip_cam, last_ip_cam_time
    ip_value = request.args.get('set_ip')
    
    if ip_value:
        last_ip_cam = ip_value
        last_ip_cam_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        return jsonify({"status": "success", "ip": ip_value, "timestamp": last_ip_cam_time}), 200
    else:
        return jsonify({"status": "error", "message": "No IP provided"}), 400

@app.route('/get_ip', methods=['GET'])
def get_ip():
    if last_ip_cam:
        return jsonify({
            "status": "success",
            "last_ip": last_ip_cam,
            "timestamp": last_ip_cam_time
        }), 200
    else:
        return jsonify({"status": "error", "message": "No IP set yet"}), 404

@app.route('/set_ip_motor', methods=['GET'])
def set_ip_motor():
    global last_ip_motor, last_ip_motor_time
    ip_value = request.args.get('set_ip_motor')
    
    if ip_value:
        last_ip_motor = ip_value
        last_ip_motor_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        return jsonify({"status": "success", "ip": ip_value, "timestamp": last_ip_motor_time}), 200
    else:
        return jsonify({"status": "error", "message": "No IP provided"}), 400

@app.route('/get_ip_motor', methods=['GET'])
def get_ip_motor():
    if last_ip_motor:
        return jsonify({
            "status": "success",
            "last_ip_motor": last_ip_motor,
            "timestamp": last_ip_motor_time
        }), 200
    else:
        return jsonify({"status": "error", "message": "No motor IP set yet"}), 404

@app.route('/', methods=['GET'])
def home():
    html = """
    <!DOCTYPE html>
    <html lang="es">
    <head>
        <meta charset="UTF-8">
        <title>Servidor de IPs - ESP32</title>
        <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css">
        <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.10.5/font/bootstrap-icons.css">
        <style>
            body {
                background-color: #f8f9fa;
            }
            .card {
                box-shadow: 0 4px 8px rgba(0,0,0,0.1);
                border-radius: 15px;
            }
            .header-icon {
                font-size: 2rem;
                color: #0d6efd;
            }
        </style>
    </head>
    <body>
        <div class="container py-5">
            <div class="text-center mb-4">
                <h1><i class="bi bi-wifi header-icon"></i> Monitor de IPs ESP32</h1>
                <p class="text-muted">Consulta en tiempo real las últimas IPs registradas</p>
            </div>

            <div class="row g-4">
                <div class="col-md-6">
                    <div class="card p-4">
                        <h4><i class="bi bi-camera-video"></i> ESP32-CAM</h4>
                        <p class="mb-1"><strong>Última IP:</strong> {{ last_ip_cam if last_ip_cam else "No registrada aún" }}</p>
                        <p><strong>Actualizada:</strong> {{ last_ip_cam_time if last_ip_cam_time else "N/A" }}</p>
                        <span class="badge bg-primary">Cámara</span>
                    </div>
                </div>
                <div class="col-md-6">
                    <div class="card p-4">
                        <h4><i class="bi bi-cpu"></i> ESP32 Motor</h4>
                        <p class="mb-1"><strong>Última IP:</strong> {{ last_ip_motor if last_ip_motor else "No registrada aún" }}</p>
                        <p><strong>Actualizada:</strong> {{ last_ip_motor_time if last_ip_motor_time else "N/A" }}</p>
                        <span class="badge bg-success">Control de motor</span>
                    </div>
                </div>
            </div>

            <div class="text-center mt-5">
                <p class="text-muted small">Servidor Flask ejecutándose en <strong>0.0.0.0:5000</strong></p>
            </div>
        </div>
    </body>
    </html>
    """
    return render_template_string(html,
                                  last_ip_cam=last_ip_cam,
                                  last_ip_cam_time=last_ip_cam_time,
                                  last_ip_motor=last_ip_motor,
                                  last_ip_motor_time=last_ip_motor_time)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
