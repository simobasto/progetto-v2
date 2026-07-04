#pragma once
#include <chrono>
#include <iostream>
#include <thread>
#include <cstdlib>

#include "bot.hpp"
namespace scacchi {

class Gioco {
  Board b_;
  mosse m_;
  Bot bot_;

  // oggetti legati alla partita
  int modalita_{0};
  long int tempo_bianco_{600};
  long int tempo_nero_{600};
  bool partita_in_corso_{true};

  // funzioni di gioco
  void seleziona_modalita_();
  void controllo_fine_partita_(const ListaMosse& mosse_disponibili);

  void umano_turno_generalizzato_(long int& tempo_rimasto, std::string colore);
  void umano_turno_bianco_();
  void umano_turno_nero_ ();
  
  void bot_turno_generalizzato_(long int& tempo_rimasto, std::string colore);
  void bot_bianco_();
  void bot_nero_();

 public:
  Gioco();
  void giochiamo();
};
}  // namespace scacchi