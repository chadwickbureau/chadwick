/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2013, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/python/chadwick/scorebook.i
 * SWIG interface wrapper for Chadwick Python library - scorebooks
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

//==========================================================================
//            Wrapping and extending CWScorebook as Scorebook
//==========================================================================

%rename(Scorebook) CWScorebook;
#if SWIG_VERSION < 0x020000
%extend CWScorebook {
  CWScorebook(void)    
#else
%extend cw_scorebook_struct {
  cw_scorebook_struct(void)
#endif
{ return cw_scorebook_create(); }
  
#if SWIG_VERSION < 0x020000
  ~CWScorebook()
#else
  ~cw_scorebook_struct()
#endif
{ cw_scorebook_cleanup(self);  free(self); }

  void write(FILE *file)       { cw_scorebook_write(self, file); }

%pythoncode %{
  def add(self, game, noclobber=True):
    if noclobber and game.key_game in self:
      raise KeyError("Game '%s' is already in scorebook" % game.key_game)
    old_game = cw_scorebook_remove_game(self, game.key_game)
    if old_game is not None:
      # This insures the underlying C object will be deleted
      # when it goes out of scope at the end of the function    
      old_game.thisown = True
    cw_scorebook_insert_game(self, game)
    game.thisown = False

  def remove(self, key_game):
    game = cw_scorebook_remove_game(self, key_game)
    if game is None:
      raise KeyError("Game '%s' is not in scorebook" % key_game)
    # The game is no longer managed by the scorebook, so the
    # Python object must take responsibility
    game.thisown = True
    return game

  def keys(self):
    return [ g.key_game for g in self ]
 
  def __getitem__(self, key_game):
    g = self.first_game
    while g is not None:
      if g.key_game == key_game:
        return g
      g = g.next
    raise KeyError("Game '%s' is not in scorebook" % key_game)
    
  def __delitem__(self, key_game):  self.remove(key_game)

  def __contains__(self, x):
    return x in self.keys() or x in [ g for g in self ]

  def __len__(self):
    g = self.first_game
    i = 0
    while g is not None:
      i += 1
      g = g.next
    return i

  def __iter__(self):
    g = self.first_game
    while g is not None:  
      yield g
      g = g.next
%}
};

%pythoncode %{
def create_scorebook():
  return cw_scorebook_create()
%}

%pythoncode %{
def read_scorebook(f):
  book = cw_scorebook_create()
  if cw_scorebook_read(book, f) == -1:
    raise IOError("Unable to load scorebook from file")
  return book
%}
