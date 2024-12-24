from flask import Blueprint, render_template, request, redirect, url_for, flash, jsonify,current_app
from flask_login import login_required, current_user
from ..models import db, Attractions, Sessions

bp = Blueprint('admin', __name__, url_prefix='/admin')

@bp.before_request
@login_required
def admin_check():
    if current_user.role != 'admin':
        flash("Access denied.")
        return redirect(url_for('auth.login'))

@bp.route('/attractions', methods=['GET','POST'])
def manage_attractions():
    if request.method == 'POST':
        # Зчитування даних з форми
        validation_errors = validate_attraction_data(request.form)
        if validation_errors:
            return jsonify({'error': validation_errors}), 400

        # Створення нового атракціону
        attraction = Attractions(
            name=request.form.get('name'),
            description=request.form.get('description'),
            availability=request.form.get('availability') == 'True',
            max_seats=int(request.form.get('max_seats'))
        )
        db.session.add(attraction)
        db.session.commit()
        flash('Attraction added successfully!')
        return '', 200
    page = request.args.get('page', 1, type=int)
    attractions = Attractions.query.paginate(page=page, per_page=current_app.config["ATTRACTIONS_PER_PAGE"], error_out=False).items

    return render_template(
        'admin/manage_attractions.html',
        attractions=attractions,
        current_page=page,
        ATTRACTIONS_PER_PAGE=current_app.config["ATTRACTIONS_PER_PAGE"]
    )

# Редагування атракціону
@bp.route('/attractions/<int:attraction_id>', methods=['PUT'])
def edit_attraction(attraction_id):
    attraction = Attractions.query.get_or_404(attraction_id)
    validation_errors = validate_attraction_data(request.form)
    if validation_errors:
        return jsonify({'error': validation_errors}), 400

    # Оновлення даних атракціону
    attraction.name = request.form.get('name')
    attraction.description = request.form.get('description')
    attraction.availability = request.form.get('availability') == 'True'
    attraction.max_seats = int(request.form.get('max_seats'))
    db.session.commit()
    flash('Attraction updated successfully!')
    return '', 200

# Видалення атракціону
@bp.route('/attractions/<int:attraction_id>', methods=['GET','DELETE'])
def delete_attraction(attraction_id):
    attraction = Attractions.query.get_or_404(attraction_id)

    if request.method == 'GET':
        # Повертаємо дані сесії у форматі JSON
        return jsonify({
            'id': attraction.id,
            'name': attraction.name,
            'description': attraction.description,
            'availability': attraction.availability,
            'max_seats': attraction.max_seats
        })
    
    db.session.delete(attraction)
    db.session.commit()
    flash('Attraction deleted successfully!')
    return '', 200

def validate_attraction_data(form):
    name = form.get('name')
    max_seats = form.get('max_seats')

    if not name or len(name.strip()) == 0:
        return 'Name is required.'
    try:
        max_seats = int(max_seats)
        if max_seats < 1:
            return 'Max seats must be a positive integer.'
    except ValueError:
        return 'Max seats must be a positive integer.'

    return None

# @bp.route('/sessions', methods=['GET', 'POST'])
# def manage_sessions():
#     if request.method == 'POST':
#         # 5. Added validation for session data
#         validation_errors = validate_session_data(request.form)
#         if validation_errors:
#             return jsonify({'error': validation_errors}), 400

#         session = Sessions(
#             attraction_id=int(request.form['attraction_id']),
#             start_time=request.form['start_time'],
#             end_time=request.form['end_time'],
#             available_spots=int(request.form['available_spots'])
#         )
#         db.session.add(session)
#         db.session.commit()
#         flash('Session added successfully!')
#         return '', 200

#     # 6. Improved database querying with relationships
#     sessions = Sessions.query.order_by(Sessions.start_time.desc()).all()
#     return render_template('admin/manage_sessions.html', sessions=sessions)

# def validate_session_data(form):
#     attraction_id = form.get('attraction_id')
#     start_time = form.get('start_time')
#     end_time = form.get('end_time')
#     available_spots = form.get('available_spots')

#     if not attraction_id or not start_time or not end_time or not available_spots:
#         return 'All fields are required.'
#     try:
#         int(attraction_id)
#         int(available_spots)
#     except ValueError:
#         return 'Attraction ID and available spots must be valid integers.'

#     return None

@bp.route('/sessions', methods=['GET', 'POST'])
@login_required
def manage_sessions():
    if request.method == 'POST':
        # Додавання або оновлення сесії
        data = request.form
        if 'id' in data and data['id']:
            session = Sessions.query.get(int(data['id']))
            if not session:
                return jsonify({'error': 'Session not found'}), 404
        else:
            session = Sessions()

        session.attraction_id = int(data['attraction_id'])
        session.start_time = data['start_time']
        session.available_spots = int(data['available_spots'])
        session.capacity = session.attraction.max_seats
        db.session.add(session)
        db.session.commit()
        return '', 200

    # Відображення сесій
    page = request.args.get('page', 1, type=int)
    sessions = Sessions.query.order_by(Sessions.start_time).paginate(page=page, per_page=current_app.config["SESSIONS_PER_PAGE"], error_out=False)
    attractions = Attractions.query.all()
    return render_template(
        'admin/manage_sessions.html',
        sessions=sessions.items,
        attractions=attractions,
        current_page=page,
        total_pages=sessions.pages,
        SESSIONS_PER_PAGE=current_app.config["SESSIONS_PER_PAGE"]
    )

@bp.route('/sessions/<int:session_id>', methods=['GET', 'DELETE'])
@login_required
def session_details(session_id):
    session = Sessions.query.get_or_404(session_id)

    if request.method == 'GET':
        # Повертаємо дані сесії у форматі JSON
        return jsonify({
            'id': session.id,
            'attraction_id': session.attraction_id,
            'start_time': session.start_time.isoformat(),
            'end_time': session.end_time.isoformat(),
            'available_spots': session.available_spots,
            'duration': session.duration
        })

    if request.method == 'DELETE':
        # Видаляємо сесію
        db.session.delete(session)
        db.session.commit()
        flash('Session deleted successfully!')
        return '', 200
    
@bp.route('/sessions/<int:session_id>', methods=['PUT'])
@login_required
def update_session(session_id):
    session = Sessions.query.get_or_404(session_id)

    # Оновлення даних сесії
    session.attraction_id = request.form.get('attraction_id', session.attraction_id)
    session.start_time = request.form.get('start_time', session.start_time)

    try:
        session.duration = int(request.form.get('duration', session.duration))
    except ValueError:
        return jsonify({'error': 'Duration must be a number.'}), 400
    try:
        session.available_spots = int(request.form.get('available_spots', session.available_spots))
    except ValueError:
        return jsonify({'error': 'Available spots must be a number.'}), 400

    db.session.commit()
    return '', 200