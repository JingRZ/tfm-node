import tkinter as tk
from tkinter import ttk
from ttkthemes import ThemedTk
import subprocess
import os

def on_submit():
    ssid_value = ssid.get()
    pwd_value = pwd.get()
    sec_ver_value = sec_ver.get()
    pop_value = pop.get()
    transport_value = transport.get()
    custom_data_value = custom_data_text.get("1.0", tk.END).strip().replace('\n', '').replace('\t', '')
    print(f"SSID: {ssid_value}" + "\n" + f"Password: {pwd_value}" + "\n" + f"Security Version: {sec_ver_value}" + "\n" + f"POP: {pop_value}" + "\n" + f"Custom Data: {custom_data_value}")

    esp_prov_path = os.getenv("IDF_PATH") + "/tools/esp_prov/esp_prov.py"
    cmd = ['python3', esp_prov_path, '--transport', transport_value, '--ssid', ssid_value, '--passphrase', pwd_value, '--sec_ver', sec_ver_value, '--pop', pop_value, '--custom_data', custom_data_value]

    result = subprocess.run(cmd, capture_output=True, text=True)
    print(result.stdout)
    

'''
python3 esp_prov.py 
    --transport softap 
    --ssid ssid_here 
    --passphrase pwd_here 
    --sec_ver 1 
    --pop abcd1234 
    --custom_data '{"tb":{"token":"da_token_here", "broker_url": "da_broker_url_here"}}'
'''

root = ThemedTk(theme="adapta")
root.title("Provision UI")

# Wifi credentials

tk.Label(root, text="SSID").grid(row=0, column=0)
tk.Label(root, text="Password").grid(row=0, column=2)

ssid = tk.Entry(root)
pwd = tk.Entry(root)

ssid.grid(row=0, column=1)
pwd.grid(row=0, column=3)

ssid.insert(0, "MiFibra-ED4A")
pwd.insert(0, "wMUS3eAS")


# Security version and POP
tk.Label(root, text="Security Version").grid(row=2, column=0)
tk.Label(root, text="POP").grid(row=2, column=2)
tk.Label(root, text="Transport").grid(row=2, column=4)

sec_ver = ttk.Combobox(root, values=["0", "1", "2"])
pop = tk.Entry(root)
transport = ttk.Combobox(root, values=["softap", "ble"])

sec_ver.grid(row=2, column=1)
pop.grid(row=2, column=3)
transport.grid(row=2, column=5)

sec_ver.current(1)
pop.insert(0, "abcd1234")
transport.current(0)


# Custom data
tk.Label(root, text="Custom Data").grid(row=4, column=0)
custom_data_text = tk.Text(root, height=10, width=70)
custom_data_text.grid(row=4, column=1, columnspan=4)
default_data = '{\n' \
             '\t"tag": {\n' \
             '\t\t"writepwd": "1q2w3e4r5t6y7u8i"\n' \
             '\t},\n' \
             '\t"tb": {\n' \
             '\t\t"token": "fvr2kvobg3yjrj0ktez0",\n' \
             '\t\t"broker_url": "thingsboard.jrzhao.com"\n' \
             '\t}\n' \
             '}'
custom_data_text.insert(tk.END, default_data)


# Create a submit button
submit_button = tk.Button(root, text="Submit", command=on_submit)
submit_button.grid(row=8, columnspan=2)

# Start the main loop
root.mainloop()
