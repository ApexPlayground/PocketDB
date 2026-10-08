
import socket
import threading

HOST = "127.0.0.1"
PORT = 6379


def send_command(client, command):
    client.sendall((command + "\r\n").encode())
    return client.recv(4096).decode()


# String, list and hash tests
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
    client.settimeout(5)
    client.connect((HOST, PORT))

    assert send_command(client, "FLUSHALL") == "+OK\r\n"

    # Strings
    assert send_command(client, "PING") == "+PONG\r\n"
    assert send_command(client, "SET name Divine") == "+OK\r\n"
    assert send_command(client, "GET name") == "$6\r\nDivine\r\n"
    assert send_command(client, "DEL name") == ":1\r\n"
    assert send_command(client, "GET name") == "$-1\r\n"

    print("String commands passed")

    # Lists
    assert send_command(client, "LPUSH names Alice") == ":1\r\n"
    assert send_command(client, "RPUSH names Bob") == ":2\r\n"
    assert send_command(client, "LLEN names") == ":2\r\n"
    assert send_command(client, "LINDEX names 0") == "$5\r\nAlice\r\n"
    assert send_command(client, "LPOP names") == "$5\r\nAlice\r\n"
    assert send_command(client, "RPOP names") == "$3\r\nBob\r\n"

    print("List commands passed")

    # Hashes
    assert send_command(client, "HSET user name Divine") == ":1\r\n"
    assert send_command(client, "HGET user name") == "$6\r\nDivine\r\n"
    assert send_command(client, "HEXISTS user name") == ":1\r\n"
    assert send_command(client, "HLEN user") == ":1\r\n"
    assert send_command(client, "HDEL user name") == ":1\r\n"
    assert send_command(client, "HEXISTS user name") == ":0\r\n"

    print("Hash commands passed")


# Multithreading tests
errors = []


def test_client(client_id, shared=False):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
            client.settimeout(5)
            client.connect((HOST, PORT))

            for i in range(100):
                if shared:
                    key = "shared_key"
                    value = f"client{client_id}"
                else:
                    key = f"client{client_id}_{i}"
                    value = "hello"

                assert send_command(client, f"SET {key} {value}") == "+OK\r\n"

                response = send_command(client, f"GET {key}")

                if shared:
                    valid_responses = {
                        f"${len(f'client{n}')}\r\nclient{n}\r\n"
                        for n in range(10)
                    }
                    assert response in valid_responses
                else:
                    assert response == "$5\r\nhello\r\n"

    except Exception as e:
        errors.append((client_id, str(e)))


def run_threads(shared=False):
    threads = []
    errors.clear()

    for i in range(10):
        t = threading.Thread(target=test_client, args=(i, shared))
        threads.append(t)
        t.start()

    for t in threads:
        t.join()

    assert not errors, f"Multithreading test failed: {errors}"


run_threads()
print("Unique key concurrency test passed")

run_threads(shared=True)
print("Shared key concurrency test passed")

# Clear test data
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
    client.connect((HOST, PORT))
    assert send_command(client, "FLUSHALL") == "+OK\r\n"

print("Database cleared")
print("All server tests passed!")
