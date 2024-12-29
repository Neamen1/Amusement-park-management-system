import socket
import threading
import time
import matplotlib.pyplot as plt
import random
# Параметри сервера
HOST = '127.0.0.1'  # Адреса сервера
PORT = 8080  # Порт сервера
words_to_search = ["but", "also", "key", "keys", "mouse", "monitor", "soda", "water", "coffee", "tea",
                   "display", "film", "movie", "test", "juice", "approx", "pen", 
                   "search", "find", "query","text", "good", "search", "find", "query",
                   "table","furniture", "sad","glad", "noise", "light", "bright","happy"]
#MESSAGE = "SEARCH text\n"  # Повідомлення, яке надсилається серверу

# Глобальні змінні для збирання даних
response_times = []  # Час відповіді сервера
successful_responses = 0  # Кількість успішних відповідей
total_requests = 0  # Загальна кількість запитів
long_responses_count=0
lock = threading.Lock()  # Для синхронізації доступу до змінних

def send_request():
    global successful_responses, total_requests, long_responses_count
    search_phrase ="SEARCH_PHRASE "#+random.choice(words_to_search)+"\n"
    for _ in range(1, 4):
        search_phrase+=(random.choice(words_to_search))+" "
    start_time = time.time()
    try:
        with socket.create_connection((HOST, PORT)) as sock:
            sock.sendall(search_phrase.encode('utf-8'))
            response = sock.recv(4096)
            response_time = time.time() - start_time

            with lock:
                response_times.append(response_time)
                total_requests += 1
                if(response_time>2):
                    long_responses_count+=1
                if response:
                    successful_responses += 1
    except Exception as e:
        with lock:
            total_requests += 1  # Враховуємо запит, навіть якщо він помилковий

def stress_test(num_clients, duration):
    global successful_responses, total_requests
    threads = []
    start_time = time.time()

    while time.time() - start_time < duration:
        for _ in range(num_clients):
            thread = threading.Thread(target=send_request)
            threads.append(thread)
            thread.start()

        # Очікуємо завершення всіх потоків
        for thread in threads:
            thread.join()

    # Підрахунок результатів
    # print(f"Total Requests: {total_requests}")
    # print(f"Successful Responses: {successful_responses}")
    # print(f"Average Response Time: {sum(response_times) / len(response_times):.4f} seconds")

    # Побудова графіка
    #plot_results()

def plot_results():
    # Побудова гістограми затримки
    plt.figure(figsize=(10, 5))
    plt.hist(response_times, bins=30, color='blue', alpha=0.7, edgecolor='black')
    plt.title("Response Times Distribution")
    plt.xlabel("Response Time (seconds)")
    plt.ylabel("Frequency")
    plt.grid(True)

    # Відображення графіка
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    # num_clients = 2000  # Кількість одночасних клієнтів
    # duration = 10  # Тривалість тесту в секундах
    # stress_test(num_clients, duration)

    #for i in range(2000, 5000,100):
    response_times=[]
    successful_responses=0
    total_requests=0
    long_responses_count=0
    #print(i)
    realstart_time = time.time()
    stress_test(20000, 10)
    realend_time = time.time()
    #if(long_responses_count>0):
    # Підрахунок результатів
    print(f"__Total Requests: {total_requests}")
    print(f"Successful Responses: {successful_responses}")
    print(f"Average Response Time: {sum(response_times) / len(response_times):.4f} seconds")
    print(f"Long Responses: {long_responses_count}")
    print(f"real time {realend_time-realstart_time}")
    plot_results()
        

    # num_clients = 2000  # Кількість одночасних клієнтів
    # duration = 10  # Тривалість тесту в секундах
    # stress_test(num_clients, duration)

