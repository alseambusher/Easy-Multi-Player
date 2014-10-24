from lib import db
session=db.new_session()
#db.setup()
#user={'fname':'alse','lname':'ambusher','game':'aoe'}
#db.new_game_server(user)
#session.commit()
#
user={'fname':'gamer3','lname':'pro3'}
db.new_game_client(user,"b8b15ab61f3fe23b968bf72762ba3d77")
session.commit()
