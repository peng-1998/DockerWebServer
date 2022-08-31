from flask import Blueprint, flash, render_template, redirect, request
from utils import TestPasswd

login_bp = Blueprint('login_bp', 'login')


@login_bp.route('/login', methods=('GET', 'POST'))
def login():
    if request.method == 'POST':
        if request.form['key'] == 'login':
            user_name = request.form.get('account')
            passward = request.form.get('password')
            hold_login = request.form.get('holdlogin')
            if TestPasswd(user_name, passward):
                resp = redirect('/')
                if hold_login == 'True':
                    resp.set_cookie('uname', user_name, 60 * 60 * 24 * 30)
                else:
                    resp.set_cookie('uname', user_name, 60 * 60)
                return resp
            else:
                flash("该用户名和密码不存在")
                return redirect('/login')
    return render_template('login.html')
