import tkinter as tk
from tkinter import filedialog, messagebox
import socket
import re

#host=localhost dbname=amusement_database port=5432 user=admin password=1221

def send_request(host, port, message):
    try:
        with socket.create_connection((host, port)) as sock:
            sock.sendall(message.encode('utf-8'))
            response = sock.recv(4096).decode('utf-8')
            return response
    except Exception as e:
        return f"Error: {e}"

def on_search():
    term = entry_param.get()
    if not term:
        messagebox.showwarning("Input Required", "Please enter a search term.")
        return
    response = send_request(entry_host.get(), int(entry_port.get()), f"SEARCH {term}")
    append_response(f"SEARCH Response:\n{response}\n\n")

def on_search_phrase():
    phrase = entry_param.get()
    if not phrase:
        messagebox.showwarning("Input Required", "Please enter a search phrase")
        return
    response = send_request(entry_host.get(), int(entry_port.get()), f"SEARCH_PHRASE {phrase}")
    append_response(f"SEARCH_PHRASE Response:\n{response}\n\n")

def on_clear():
    response = send_request(entry_host.get(), int(entry_port.get()), "CLEAR_INDEX")
    append_response(f"CLEAR_INDEX Response:\n{response}\n\n")

def on_index_db():
    conn_info = entry_param.get()
    if not conn_info:
        messagebox.showwarning("Input Required", "Please enter the connection info.")
        return
    response = send_request(entry_host.get(), int(entry_port.get()), f"INDEX_DB {conn_info}\n")
    append_response(f"INDEX_DB Response:\n{response}\n\n")

def on_add_file():
    file_path = filedialog.askopenfilename()
    if file_path:
        response = send_request(entry_host.get(), int(entry_port.get()), f"ADD_FILE {file_path}\n")
        append_response(f"ADD_FILE Response:\n{response}\n\n")

def on_add_directories():
    dir_path = filedialog.askdirectory()
    num_threads = entry_param.get()

    # Validate the input format
    if not re.fullmatch(r"\d+", num_threads):
        messagebox.showwarning("Invalid Input", "Input must be in the format 'numThreads', where first is a non-negative integer")
        return
    response = send_request(entry_host.get(), int(entry_port.get()), f"addDocsFromDirectory {num_threads},{dir_path}\n")
    append_response(f"addDocsFromDirectory Response:\n{response}\n\n")

def on_save_index():
    file_path = filedialog.asksaveasfilename(defaultextension=".json", filetypes=[("JSON files", "*.json")])
    if file_path:
        response = send_request(entry_host.get(), int(entry_port.get()), f"SAVE_INDEX {file_path}\n")
        append_response(f"SAVE_INDEX Response:\n{response}\n\n")

def on_load_index():
    file_path = filedialog.askopenfilename(filetypes=[("JSON files", "*.json")])
    if file_path:
        response = send_request(entry_host.get(), int(entry_port.get()), f"LOAD_INDEX {file_path}\n")
        append_response(f"LOAD_INDEX Response:\n{response}\n\n")



def on_print_index():
    term = entry_param.get()
    response = send_request(entry_host.get(), int(entry_port.get()), f"PRINT_INDEX {term}")
    append_response(f"PRINT_INDEX Response:\n{response}\n\n")

def append_response(response):
    text_output.insert(tk.END, response)
    text_output.see(tk.END)  # Automatically scroll to the latest text

# Create the main window
root = tk.Tk()
root.title("Server Request Interface")

# Host and Port Frame
frame_connection = tk.Frame(root)
frame_connection.pack(pady=5)

label_host = tk.Label(frame_connection, text="Host:")
label_host.pack(side=tk.LEFT)
entry_host = tk.Entry(frame_connection, width=15)
entry_host.insert(0, "127.0.0.1")
entry_host.pack(side=tk.LEFT, padx=5)

label_port = tk.Label(frame_connection, text="Port:")
label_port.pack(side=tk.LEFT)
entry_port = tk.Entry(frame_connection, width=5)
entry_port.insert(0, "8080")
entry_port.pack(side=tk.LEFT, padx=5)

# Parameter Frame
frame_param = tk.Frame(root)
frame_param.pack(pady=5)

label_param = tk.Label(frame_param, text="Parameter:")
label_param.pack(side=tk.LEFT)
entry_param = tk.Entry(frame_param, width=40)
entry_param.pack(side=tk.LEFT, padx=5)

# Buttons Frame
frame_buttons = tk.Frame(root)
frame_buttons.pack(pady=10)

btn_search = tk.Button(frame_buttons, text="SEARCH", command=on_search)
btn_search.pack(side=tk.LEFT, padx=5)

btn_search = tk.Button(frame_buttons, text="SEARCH_PHRASE", command=on_search_phrase)
btn_search.pack(side=tk.LEFT, padx=5)

btn_clear = tk.Button(frame_buttons, text="CLEAR_INDEX", command=on_clear)
btn_clear.pack(side=tk.LEFT, padx=5)

btn_index_db = tk.Button(frame_buttons, text="INDEX_DB", command=on_index_db)
btn_index_db.pack(side=tk.LEFT, padx=5)

btn_add_file = tk.Button(frame_buttons, text="ADD_FILE", command=on_add_file)
btn_add_file.pack(side=tk.LEFT, padx=5)

btn_add_file = tk.Button(frame_buttons, text="ADD_FROM_DIRECTORY", command=on_add_directories)
btn_add_file.pack(side=tk.LEFT, padx=5)

btn_save_index = tk.Button(frame_buttons, text="SAVE_INDEX", command=on_save_index)
btn_save_index.pack(side=tk.LEFT, padx=5)

btn_load_index = tk.Button(frame_buttons, text="LOAD_INDEX", command=on_load_index)
btn_load_index.pack(side=tk.LEFT, padx=5)

btn_print_index = tk.Button(frame_buttons, text="PRINT_INDEX", command=on_print_index)
btn_print_index.pack(side=tk.LEFT, padx=5)

# Output Text Frame
frame_output = tk.Frame(root)
frame_output.pack(pady=10)

text_output = tk.Text(frame_output, height=25, width=80)
text_output.pack()

# Run the application
root.mainloop()
