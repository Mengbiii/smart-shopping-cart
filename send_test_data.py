import requests
from datetime import datetime, timedelta
import random
import json

url = "http://127.0.0.1:5000/add_cart_data"

# Sample products with price and weight (kg)
products = [
    {"name": "Milk", "price": 2.5, "weight": 1.0},
    {"name": "Apple", "price": 1.0, "weight": 0.2},
    {"name": "Chips", "price": 3.0, "weight": 0.3},
    {"name": "Banana", "price": 1.2, "weight": 0.25},
    {"name": "Bread", "price": 2.5, "weight": 0.5},
    {"name": "Butter", "price": 2.0, "weight": 0.15},
    {"name": "Eggs", "price": 3.0, "weight": 0.6},
    {"name": "Yogurt", "price": 2.0, "weight": 0.4},
    {"name": "Orange Juice", "price": 4.0, "weight": 1.2},
    {"name": "Cereal", "price": 3.5, "weight": 0.8}
]

# Sample customers
customers = ["Amit", "Neha", "Ravi", "Zara", "Liam", "Tina", "John", "Sara"]

# Base timestamp
base_time = datetime.now().replace(minute=0, second=0, microsecond=0)

# Scanned count stats
scannedCounts = {}

for i in range(10):
    customer = random.choice(customers)
    num_items = random.randint(2, 4)
    chosen = random.sample(products, num_items)
    timestamp = (base_time.replace(hour=9) + timedelta(hours=i)).isoformat()

    # Total price calculation
    total = sum(item["price"] for item in chosen)

    # Track scanned counts
    for item in chosen:
        scannedCounts[item["name"]] = scannedCounts.get(item["name"], 0) + 1

    # Prepare payload
    payload = {
        "customer": customer,
        "items": chosen,
        "total": total,
        "timestamp": timestamp
    }

    # Send POST request
    res = requests.post(url, json=payload)
    print(f"[{customer}] Sent cart at {timestamp[11:16]} | Items: {len(chosen)} -> {res.status_code}")

# Optional: write scannedCounts to JS
with open("mock_scanned_data.js", "w") as f:
    f.write("const scannedCounts = ")
    json.dump(scannedCounts, f, indent=2)
    f.write(";")
