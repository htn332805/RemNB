import pexpect
import time


def wait_for_prompt(process, expected_prompt, count=None, meta_char=None):
    """
    Wait for expected prompt in a pexpect process with retry logic

    :param process: pexpect.spawn process object (SSH/telnet connection)
    :param expected_prompt: Regex pattern to expect as prompt
    :param count: Optional max retry attempts (returns 0 if not found)
    :param meta_char: Optional control character to send before checking
    :return: Matched prompt string or 0 if count reached
    """
    attempts = 0

    while True:
        if meta_char:
            print(
                f"Caller is sending {meta_char} to terminal current running process..."
            )
            process.send(meta_char)

        try:
            process.expect([expected_prompt], timeout=3)
            return process.match.group()  # .decode("utf-8")
        except pexpect.TIMEOUT:
            print(
                f"With attempt {attempts}. Can't match the prompt and time out reached...."
            )
            process.send("\n\r")
            attempts += 1
            if count and attempts >= count:
                return 0


# Establish SSH connection
conn = pexpect.spawn(
    "./connsat_client/connsat_client -p 50000", timeout=10, encoding="utf-8"
)
time.sleep(1)
conn.send("\n\n\r")
# Wait for login prompt with max 5 attempts and meta character
prompt = wait_for_prompt(
    process=conn,
    expected_prompt=r"#] ",  # Regex for # or $ prompt
    # count=5,``
    meta_char="\x03",  # Ctrl+C to interrupt any current command
)

if prompt:
    print(f"Connected to device with prompt: {prompt}")
else:
    print("Failed to detect prompt within the specified maximum retries")
