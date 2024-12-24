from . import db
from flask_login import UserMixin

class Users(UserMixin, db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(50), unique=True, nullable=False)
    email = db.Column(db.String(100), nullable=False)
    password = db.Column(db.String(255), nullable=False)
    role = db.Column(db.String(50), default='user')  # 'admin' or 'user'
    created_at = db.Column(db.TIMESTAMP, server_default=db.func.now())
    
    def __repr__(self):
        return f'<User {self.username}>'

class Attractions(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100), nullable=False)
    description = db.Column(db.String(255))
    availability = db.Column(db.Boolean, default=True)
    max_seats = db.Column(db.Integer, nullable=False)
    created_at = db.Column(db.TIMESTAMP, server_default=db.func.now())

class Sessions(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    attraction_id = db.Column(db.Integer, db.ForeignKey('attractions.id'), nullable=False)
    start_time = db.Column(db.TIMESTAMP, nullable=False)
    end_time = db.Column(db.TIMESTAMP)
    capacity = db.Column(db.Integer, nullable=False)
    available_spots = db.Column(db.Integer, nullable=False)
    created_at = db.Column(db.TIMESTAMP, server_default=db.func.now())
    duration = db.Column(db.Integer, nullable=False)

    # Relationship to Attractions
    attraction = db.relationship('Attractions', backref='sessions')


class Bookings(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.Integer, nullable=False)
    session_id = db.Column(db.Integer, db.ForeignKey('sessions.id'), nullable=False)
    booking_time = db.Column(db.TIMESTAMP, server_default=db.func.now(), nullable=False)
    status = db.Column(db.String(20), default='Pending')  # 'Pending', 'Confirmed', 'Cancelled'
    seats = db.Column(db.Integer, nullable=False)
