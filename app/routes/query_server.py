import socket
from jsonify import convert

def send_request_to_cpp_server(host, port, message):
    """
    Sends a request to the C++ server and returns the response.
    """
    try:
        with socket.create_connection((host, port)) as sock:
            sock.sendall(message.encode('utf-8'))
            response = sock.recv(4096).decode('utf-8')
            return response
    except Exception as e:
        return 400, e

def query_cpp_server(search_query):

    
    host = '127.0.0.1'  # Default to localhost
    port = 8080         # Default to port 8080

    if not search_query:
        return 400, 'Query is required'

    # Sending query to the C++ server
    response = send_request_to_cpp_server(host, port, f"SEARCH_PHRASE {search_query}")
    response = [int(x) for x in response[3:].split(',')[:-1]]  # Split the response into a list of IDs
    print(response)
    return 200, response


if __name__ == "__main__":
    print(query_cpp_server("text"))