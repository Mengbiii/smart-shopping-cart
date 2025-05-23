from flask import Flask, request, jsonify, render_template
import sqlite3
from datetime import datetime

app = Flask(__name__)
DB = 'database.db'

def init_db():
    with sqlite3.connect(DB) as conn:
        cursor = conn.cursor()
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS transactions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                customer TEXT,
                product TEXT,
                price REAL,
                weight REAL,
                cart_id TEXT,
                timestamp TEXT
            )
        ''')

        # 商品信息表
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS products (
                uid TEXT PRIMARY KEY,
                name TEXT,
                price REAL,
                is_fruit INTEGER
            )
        ''')




        # 插入初始商品数据（如果表为空）
        cursor.execute("SELECT COUNT(*) FROM products")
        if cursor.fetchone()[0] == 0:
            products = [
                ("041F8142E81491", "Nuts", 25.0, 1),
                ("B9BD18C9", "Apple", 4.2, 1),
                ("0420D6BAFD1691", "water", 3.8, 0)
            ]
            cursor.executemany('''
                INSERT INTO products (uid, name, price, is_fruit)
                VALUES (?, ?, ?, ?)
            ''', products)

        conn.commit()


@app.route('/')
def home():
    return 'Smart Cart Backend Running'

@app.route('/dashboard')
def dashboard():
    return render_template('dashboard.html')

@app.route('/add_cart_data', methods=['POST'])
def add_cart_data():
    data = request.get_json()
    customer = data.get('customer', 'Anonymous')
    items = data.get('items', [])
    timestamp = data.get('timestamp') or datetime.now().isoformat()
    cart_id = f"{customer[:2]}_{datetime.now().timestamp()}"[:16]

    with sqlite3.connect(DB) as conn:
        for item in items:
            name = item.get("name", "Unknown")
            price = item.get("price", 0.0)
            weight = item.get("weight", 0.0)
            conn.execute('''
                INSERT INTO transactions (customer, product, price, weight, cart_id, timestamp)
                VALUES (?, ?, ?, ?, ?, ?)
            ''', (customer, name, price, weight, cart_id, timestamp))

    return jsonify({"status": "success"}), 200


@app.route('/get_data')
def get_data():
    with sqlite3.connect(DB) as conn:
        conn.row_factory = sqlite3.Row
        cursor = conn.cursor()
        cursor.execute("SELECT * FROM transactions ORDER BY timestamp DESC")
        rows = cursor.fetchall()
        data = [dict(row) for row in rows]
        return jsonify(data)

@app.route('/get_product/<uid>', methods=['GET'])
def get_product(uid):
    with sqlite3.connect(DB) as conn:
        cursor = conn.cursor()
        cursor.execute("SELECT name, price, is_fruit FROM products WHERE uid = ?", (uid,))
        result = cursor.fetchone()

        if result:
            name, price, is_fruit = result
            return jsonify({
                "name": name,
                "price": price,
                "is_fruit": bool(is_fruit)
            }), 200
        else:
            return jsonify({"error": "Product not found"}), 404



@app.route('/add_product', methods=['POST'])
def add_product():
    data = request.get_json()
    uid = data.get("uid")
    name = data.get("name")
    price = float(data.get("price", 0))
    is_fruit = int(data.get("is_fruit", 0))

    with sqlite3.connect(DB) as conn:
        conn.execute('''
            INSERT OR REPLACE INTO products (uid, name, price, is_fruit)
            VALUES (?, ?, ?, ?)
        ''', (uid, name, price, is_fruit))
        conn.commit()

    return jsonify({"status": "success", "message": "Product added"})


@app.route('/delete_product/<uid>', methods=['DELETE'])
def delete_product(uid):
    with sqlite3.connect(DB) as conn:
        cursor = conn.cursor()
        cursor.execute("DELETE FROM products WHERE uid = ?", (uid,))
        conn.commit()

    return jsonify({"status": "success", "message": f"Product {uid} deleted."})


if __name__ == '__main__':
    init_db()
    app.run(debug=True, host='0.0.0.0')

