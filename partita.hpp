#pragma once
#include <chrono>
#include <iostream>
#include <thread>

#include "bot.hpp"
namespace scacchi {

class Gioco {
  Board b_;
  mosse m_;
  Bot bot_;

  // oggetti legati alla partita
  int modalita_{0};
  long int tempo_bianco_{900};
  long int tempo_nero_{900};
  bool partita_in_corso_{true};

  // funzioni di gioco
  void seleziona_modalita_();
  void controllo_fine_partita_(const ListaMosse& mosse_disponibili);
  void umano_turno_bianco_();
  void bot_turno_bianco_();
  void bot_turno_nero_();

 public:
  Gioco();
  void giochiamo();
};
}  // namespace scacchi