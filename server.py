from flask import Flask, request, jsonify, render_template_string
import requests

app = Flask(__name__)

last_ip = None         # IP recibida de la ESPCam
camera_response = None # Respuesta de la ESPCam

@app.route('/set_ip', methods=['GET'])
def set_ip():
    global last_ip
    ip_value = request.args.get('set_ip')
    
    if ip_value:
        last_ip = ip_value
        return jsonify({"status": "success", "ip": ip_value}), 200
    else:
        return jsonify({"status": "error", "message": "No IP provided"}), 400

@app.route('/get_ip', methods=['GET'])
def get_ip():
    if last_ip:
        return jsonify({"status": "success", "last_ip": last_ip}), 200
    else:
        return jsonify({"status": "error", "message": "No IP set yet"}), 404

@app.route('/', methods=['GET', 'POST'])
def home():
    global camera_response
    html = """
    <html>
    <head><title>Servidor Flask - ESPCam</title></head>
    <body>
        <h1>Última IP enviada por la ESPCam: {{ last_ip if last_ip else "No se ha recibido ninguna IP aún." }}</h1>

        {% if last_ip %}
        <form method="post">
            <label>Enviar IP a la ESPCam:</label><br>
            <input name="send_ip" placeholder="Escribe una IP para la cámara" required>
            <button type="submit">Enviar</button>
        </form>
        {% endif %}

        {% if camera_response %}
            <p><strong>Respuesta de la cámara:</strong> {{ camera_response }}</p>
        {% endif %}
    </body>
    </html>
    """

    if request.method == 'POST':
        target_ip = request.form.get("send_ip")
        if last_ip:
            try:
                url = f"http://{last_ip}/upload_server_camera"
                response = requests.post(url, data={"upload_server_camera": target_ip}, timeout=5)
                camera_response = f"{response.status_code} - {response.text}"
            except Exception as e:
                camera_response = f"Error al enviar: {str(e)}"
    return render_template_string(html, last_ip=last_ip, camera_response=camera_response)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
