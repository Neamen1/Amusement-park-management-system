from flask import Blueprint, render_template, request, redirect, url_for, flash
from flask_login import login_user, logout_user, login_required
from app.models import Users

bp = Blueprint('auth', __name__, url_prefix='')

@bp.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        user = Users.query.filter_by(username=username).first()
        if user and user.password == password:  # Simplified for demo; use hashing!
            login_user(user)
            return redirect(url_for('public.view_attractions'))
        else:
            flash('Invalid username or password', 'danger')
            return render_template('auth/login.html'), 401
    return render_template('auth/login.html')
    
@bp.route('/logout', methods=['POST'])
@login_required
def logout():
    logout_user()
    flash('Logged out successfully.', 'success')
    return '', 204  # Порожня відповідь з кодом успішності
