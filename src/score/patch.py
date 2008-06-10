#
# Script with miscellaneous functions for ad-hoc corrections and updates
# to game files
#

import dw
import libchadwick as cw

def ReplacePlayer(game, old_player, new_player):
    """
    Replace all references to old_player in game with new_player.
    """

    for t in [0,1]:
        for slot in xrange(1, 10):
            app = game.GetStarter(t, slot)
            if app.player_id == old_player.GetID():
                app.SetPlayerID(new_player.GetID())
                app.SetName(new_player.GetFirstName() + " " + 
                            new_player.GetLastName())
    
    it = cw.GameIterator(game)

    while it.event is not None:
        if it.event.batter == old_player.GetID():
            it.event.SetBatterID(new_player.GetID())

        for sub in it.event.Substitutes():
            if sub.player_id == old_player.GetID():
                sub.SetPlayerID(new_player.GetID())
                sub.SetName(new_player.GetFirstName() + " " +
                            new_player.GetLastName())

        it.NextEvent()
                

if __name__ == "__main__":
    book = dw.Reader("/users/arbiter/Documents/CBL2008/2008CBL.chw")

    team = book.GetTeam("TEX")

    old_player = [ x for x in team.Players()
                   if x.GetSortName() == "Barr, Brian" ][0]
    new_player = [ x for x in team.Players()
                   if x.GetSortName() == "Buchanan, Paul" ][0]

    game = book.GetGame("COR200806070")

    ReplacePlayer(game, old_player, new_player)

    dw.Writer(book, "/users/arbiter/Documents/CBL2008/2008CBL.chw")

