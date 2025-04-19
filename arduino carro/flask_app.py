from flask import Flask, request, Response
import cv2
import numpy as np
import requests


app = Flask(__name__)

# IP fija y puerto del servidor
server_ip = "192.168.1.100"
server_port = 5000
image_path = 'received_image.jpg'

# IP del receptor del mensaje de color
receiver_ip = None  # Inicialmente no está definida

# Función para obtener el color predominante
def get_dominant_color(hsv):
    colors = {
        "Rojo": ((0, 50, 50), (10, 255, 255)),
        "Verde": ((35, 50, 50), (85, 255, 255)),
        "Azul": ((100, 50, 50), (130, 255, 255)),
        "Amarillo": ((20, 50, 50), (30, 255, 255))
    }

    max_count = 0
    dominant_color = "Desconocido"

    for color_name, (lower, upper) in colors.items():
        mask = cv2.inRange(hsv, np.array(lower), np.array(upper))
        count = cv2.countNonZero(mask)
        if count > max_count:
            max_count = count
            dominant_color = color_name

    return dominant_color
@app.route('/cun', methods=['GET'])
def cun():
    return '''
    <html>
        <head><title>Bienvenida</title></head>
        <body style="text-align:center; padding-top: 100px;">
            <h1>Bienvenido</h1>
        </body>
    </html>
    '''
# Ruta para establecer la IP receptora
@app.route('/set_ip', methods=['GET'])
def set_ip():
    global receiver_ip
    ip = request.args.get('ip')
    if ip:
        receiver_ip = ip
        return {'status': 'success', 'ip_set_to': receiver_ip}
    return {'status': 'error', 'message': 'No IP provided'}

# Enviar mensaje HTTP dependiendo del color
def send_color_message(color):
    if not receiver_ip:
        return {'status': 'error', 'message': 'Receiver IP not set. Use /set_ip?ip=xxx.xxx.xxx.xxx'}

    if color == 'Rojo':
        url = f'http://{receiver_ip}/message?message=/7L'
    elif color == 'Verde':
        url = f'http://{receiver_ip}/message?message=/8P'
    elif color == 'Azul':
        url = f'http://{receiver_ip}/message?message=/9N'
    else:
        return {'status': 'error', 'message': 'Color no reconocido'}

    try:
        response = requests.get(url)
        return {'status': 'success', 'response_code': response.status_code}
    except Exception as e:
        return {'status': 'error', 'message': str(e)}

# Ruta para subir imagen y procesar color
@app.route('/upload', methods=['POST'])
def upload_image():
    try:
        img_data = request.data
        np_arr = np.frombuffer(img_data, np.uint8)
        image = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)
        cv2.imwrite(image_path, image)
        hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
        dominant_color = get_dominant_color(hsv)
        response = send_color_message(dominant_color)
        return {'status': 'success', 'color': dominant_color, 'response': response}
    except Exception as e:
        return {'status': 'error', 'message': str(e)}

# Ruta para visualizar el video
@app.route('/view', methods=['GET'])
def view_video():
    return Response(generate_video_stream(), mimetype='multipart/x-mixed-replace; boundary=frame')

# Generador del video en vivo
def generate_video_stream():
    while True:
        frame = cv2.imread(image_path)
        if frame is not None:
            ret, jpeg = cv2.imencode('.jpg', frame)
            if not ret:
                continue
            frame_bytes = jpeg.tobytes()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=0)
