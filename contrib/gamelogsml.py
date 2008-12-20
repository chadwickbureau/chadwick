# A crude script to convert Retrosheet gamelogs into SportsML

import amara
import csv
import sys
import time

def add_content_code(doc, parent, name=None, key=None, type=None):
    sCC = doc.xml_create_element(u'sports-content-code')
    parent.xml_append(sCC)
    if name is not None:
        sCC.xml_set_attribute(u'code-name', unicode(name))
    if key is not None:
        sCC.xml_set_attribute(u'code-key', unicode(key))
    if type is not None:
        sCC.xml_set_attribute(u'code-type', unicode(type))
        

def build_metadata(doc, game):
    metadata = doc.xml_create_element(u'sports-metadata')

    doc.sports_content.xml_append(metadata)

    metadata.xml_set_attribute(u'language', u'en-US')
    metadata.xml_set_attribute(u'date-time',
                               unicode(time.strftime("%Y%m%dT%H%M%S+0000",
                                                     time.gmtime())))
    metadata.xml_set_attribute(u'doc-id',
                               u'Retrosheet.%s%s%s.box' %
                               (game[6], game[0], game[1]))
    metadata.xml_set_attribute(u'revision-id',
                               u'l.mlb.com-%s-e.%s%s%s-event-stats' %
                               (game[0][:4], game[6], game[0], game[1]))
    metadata.xml_set_attribute(u'fixture-key', u'event-score')
    metadata.xml_set_attribute(u'document-class', u'event-summary')
    metadata.xml_set_attribute(u'fixture-name', u'Scoring Update')
                               
    # TODO: add sports-title

    sCC = doc.xml_create_element(u'sports-content-codes')
    metadata.xml_append(sCC)
    add_content_code(doc, sCC, name="SABR", key="sabr.org", type="publisher")
    add_content_code(doc, sCC, type="sport", key="15007000",
                     name="Baseball")
    add_content_code(doc, sCC, type="league", key="l.mlb.com",
                     name="Major League Baseball")
    add_content_code(doc, sCC, type="season", key=game[0][:4])
    add_content_code(doc, sCC, type="season-type", key="regular")
    add_content_code(doc, sCC, type="priority", key="normal")
    add_content_code(doc, sCC, type="team", key=game[3])
    add_content_code(doc, sCC, type="team", key=game[6])
    
def build_event(doc, game):    
    event = doc.xml_create_element(u'sports-event')

    doc.sports_content.xml_append(event)

    metadata = doc.xml_create_element(u'event-metadata')
    event.xml_append(metadata)

    metadata.xml_set_attribute(u'date-coverage-type', u'event')
    metadata.xml_set_attribute(u'event-key',
                               u'l.mlb.com-%s-e.%s%s%s' %
                               (game[0][:4], game[6], game[0], game[1]))
    metadata.xml_set_attribute(u'date-coverage-value', 
                               u'l.mlb.com-%s-e.%s%s%s' %
                               (game[0][:4], game[6], game[0], game[1]))
    metadata.xml_set_attribute(u'start-date-time',
                               u'%sT000000-0000' % game[0])

    metadata.xml_set_attribute(u'event-status', u'post-event')
    metadata.xml_set_attribute(u'game-of-day',
                               u'1' if game[1] == "0" else unicode(game[1]))

    eMB = doc.xml_create_element(u'event-metadata-baseball')
    metadata.xml_append(eMB)

def build_player(doc, team, key=None, full=None, event_credit=None):
    player = doc.xml_create_element(u'player')
    team.xml_append(player)

    metadata = doc.xml_create_element(u'player-metadata')
    player.xml_append(metadata)
    if key is not None:
        metadata.xml_set_attribute(u'player-key', unicode(key))

    name = doc.xml_create_element(u'name')
    metadata.xml_append(name)
    if full is not None:
        name.xml_set_attribute(u'full', unicode(full))

    if event_credit is not None:
        stats = doc.xml_create_element(u'player-stats')
        player.xml_append(stats)

        pSB = doc.xml_create_element(u'player-stats-baseball')
        stats.xml_append(pSB)

        sBP = doc.xml_create_element(u'stats-baseball-pitching')
        pSB.xml_append(sBP)
        sBP.xml_set_attribute(u'event-credit', unicode(event_credit))
        if event_credit == "save":
            sBP.xml_set_attribute(u'save-credit', unicode(event_credit))
                              

    return player


def build_team(doc, event, game, alignment):
    team = doc.xml_create_element(u'team')
    event.xml_append(team)

    metadata = doc.xml_create_element(u'team-metadata')
    team.xml_append(metadata)
    metadata.xml_set_attribute(u'alignment', unicode(alignment))
    metadata.xml_set_attribute(u'team-key',
                               unicode(game[3]) if alignment == "away"
                               else unicode(game[6]))

    stats = doc.xml_create_element(u'team-stats')
    team.xml_append(stats)

    score = int(game[9]) if alignment == "away" else int(game[10])
    oppscore = int(game[10]) if alignment == "away" else int(game[9])

    stats.xml_set_attribute(u'score', unicode(score))
    if score > oppscore:
        stats.xml_set_attribute(u'event-outcome', u'win')
        if game[93] != "":
            player = build_player(doc, team, key=game[93], full=game[94],
                                  event_credit='win')
        if game[97] != "":
            player = build_player(doc, team, key=game[97], full=game[98],
                                  event_credit='save')
    elif score < oppscore:
        stats.xml_set_attribute(u'event-outcome', u'loss')
        if game[95] != "":
            player = build_player(doc, team, key=game[95], full=game[96],
                                  event_credit='loss')

    else:
        stats.xml_set_attribute(u'event-outcome', u'tie')
    
    


def build_document(game):
    doc = amara.create_document(u'sports-content')

    build_metadata(doc, game)
    build_event(doc, game)

    build_team(doc, doc.sports_content.sports_event, game, "away")
    build_team(doc, doc.sports_content.sports_event, game, "home")

    return doc

if __name__ == "__main__":
    for game in csv.reader(file(sys.argv[1])):
        doc = build_document(game)

        print >>file("Retrosheet.%s%s%s.box.xml" % (game[6], game[0], game[1]), 'w'), doc.xml(indent=u'yes')
