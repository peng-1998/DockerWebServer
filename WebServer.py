from werkzeug.security import generate_password_hash,check_password_hash
from flask import Flask, request, g, make_response, jsonify
import bcrypt
from flask_cors import CORS
from database.SQLiteDB import SQLiteDB 

app = Flask(__name__)
CORS(app)

def get_db():
    if 'db' not in g:
        g.db = SQLiteDB(db_path="./data/database.db")
    return g.db

@app.teardown_appcontext
def close_db(error):
    if 'db' in g:
        g.db.close()

@app.route("/")
def hello_world():
    return "<p>Hello, World!</p>"

@app.route('/api/login', methods=('GET', 'POST'))
def login():
    data = request.get_json()
    user_name = data.get('username')
    print(user_name)
    database = get_db()
    user_info_list = database.get_user(search_key={'username':user_name}, return_key=['password', 'salt'])
    if user_info_list: 
        saved_password, salt = user_info_list[0]
        password = bcrypt.hashpw(data.get('password').encode(), salt.encode())
        if saved_password == password.decode():
            return make_response(jsonify(success=True, message="Login Succeed"), 200)
        else:
            return make_response(jsonify(success=False, message="Wrong Password"), 401)
    else:
        return make_response(jsonify(success=False, message="User Not Found"), 404)

@app.route('/api/register', methods=['POST'])
def register():
    database = get_db()
    data = request.get_json()
    username = data.get('username')
    hashed_password = data.get('password')
    is_user_name_exists = database.get_user(search_key={'username':username})
    if is_user_name_exists:
        return make_response(jsonify(success=False, message="用户已存在"), 409)
    else:
        salt = bcrypt.gensalt()
        hashed_password = bcrypt.hashpw(hashed_password.encode(), salt)
        database.insert_user(user={'username':username, 'password':hashed_password.decode(), 'salt':salt.decode()})
        return make_response(jsonify(success=True, message="注册成功"), 200)
    

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=9998, debug=True, threaded=True)