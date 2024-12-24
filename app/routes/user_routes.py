from flask import Blueprint, render_template, redirect, url_for, flash, request, jsonify
from flask_login import login_required, current_user
from ..models import Attractions, Bookings, Sessions, db

bp = Blueprint('user', __name__, url_prefix='/user')

@bp.route('/book/<int:session_id>', methods=['POST'])
@login_required
def book_session(session_id):
    seats = request.form.get('seats')
    if not seats or not seats.isdigit() or int(seats) < 1:
        flash("Invalid number of seats.", "danger")
        return redirect(url_for('public.view_attractions'))
    seats = int(seats)

    # Centralized session lookup
    session = Sessions.query.get(session_id)
    if not session:
        flash("Session does not exist.", "danger")
        return redirect(url_for('public.view_attractions'))

    if session.available_spots < seats:
        return jsonify({'error': 'Not enough spots available.'}), 400

    # Atomic database update for available spots
    try:
        session.available_spots -= seats
        booking = Bookings(user_id=current_user.id, session_id=session_id, seats=seats)
        db.session.add(booking)
        db.session.commit()
    except Exception as e:
        db.session.rollback()
        flash(f"An error occurred: {str(e)}", "danger")
        return redirect(url_for('public.view_attractions'))


    flash('Booking successful!', "success")
    return redirect(url_for('public.view_attractions'))

