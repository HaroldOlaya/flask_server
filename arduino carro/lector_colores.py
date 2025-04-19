from flask import Flask
import requests

app = Flask(__name__)

ip_cam = None  # Variable global para almacenar la IP de la ESP32 CAM

# Función para obtener la IP pública del servidor
def get_public_ip():
    global ip_cam  # Declaramos ip_cam como global
    try:
        response = requests.get('https://haroldolaya99.pythonanywhere.com/get_ip')
        ip_cam = response.text.strip()
        if response.status_code == 200:
            # Convertir la respuesta JSON a un diccionario
            data = response.json()
            
            # Obtener el valor de "last_ip" de la respuesta
            last_ip = data.get('last_ip')
            ip_cam = last_ip  # Guardamos la IP pública en la variable global
            print(last_ip)  # Guardamos la IP pública en la variable global
        return ip_cam  # Devolvemos la IP pública
    except Exception as e:
        print(f"Error obteniendo la IP pública: {str(e)}")
        return None

# Función para enviar la IP a la ESP32 CAM
def send_ip_to_esp32(ip):
    global ip_cam  # Declaramos ip_cam como global
    esp32_url = f"http://{ip_cam}/receive_ip"  # Ahora usamos la IP global
    payload = {'ip': ip}  # IP que queremos enviar
    try:
        response = requests.post(esp32_url, data=payload)
        if response.status_code == 200:
            print(f"IP enviada correctamente a ESP32 CAM: {ip}")
        else:
            print(f"Error al enviar la IP a ESP32 CAM: {response.status_code}")
    except Exception as e:
        print(f"Error al enviar la IP a la ESP32 CAM: {str(e)}")

# Esta función se ejecuta al iniciar la aplicación
def print_and_send_ip_on_start():
    ip = get_public_ip()
    if ip:
        print(f"La IP pública es: {ip}")
        send_ip_to_esp32(ip)  # Enviar la IP al ESP32 CAM
    else:
        print("No se pudo obtener la IP pública.")

# Llamamos a la función para obtener e imprimir la IP al arrancar la aplicación
print_and_send_ip_on_start()

@app.route('/')
def index():
    return "La IP pública ya se ha mostrado en la terminal y se ha enviado a la ESP32 CAM."

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
