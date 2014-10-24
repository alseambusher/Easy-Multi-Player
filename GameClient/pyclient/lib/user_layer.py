
# 'games':{'sess1':'data','sess2':'data2'}
# TODO this has to be reimplemented by the user
def select_game(games):
    for session_key in games:
        print session_key
    print "enter session_key to select a game:"
    session_key=str(raw_input())
    return session_key