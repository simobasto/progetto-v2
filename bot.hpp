#pragma once
#include <algorithm>
#include <array>
#include <limits>
#include <random>
#include <vector>

#include "struttura.hpp"

namespace scacchi {

class Bot {
  std::minstd_rand gen_{std::random_device{}()};
  // le tabelle sono orientate per il BIANCO

  // tabella di val. pos. RE BIANCO
  static constexpr std::array<int, 64> pst_re = {
      20,  30,  10,  0,   0,   10,  30,  20,  20,  20,  0,   0,   0,
      0,   20,  20,  -10, -20, -20, -20, -20, -20, -20, -10, -20, -30,
      -30, -40, -40, -30, -30, -20, -30, -40, -40, -50, -50, -40, -40,
      -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50,
      -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30};

  static constexpr std::array<int, 64> pst_re_endgame = {
      -50, -30, -30, -30, -30, -30, -30, -50, -30, -10, 0,   0,   0,
      0,   -10, -30, -30, 0,   20,  30,  30,  20,  0,   -30, -30, 0,
      30,  40,  40,  30,  0,   -30, -30, 0,   30,  40,  40,  30,  0,
      -30, -30, 0,   20,  30,  30,  20,  0,   -30, -30, -10, 0,   0,
      0,   0,   -10, -30, -50, -30, -30, -30, -30, -30, -30, -50};

  // tabella di val. pos. CAVALLO BIANCO
  static constexpr std::array<int, 64> pst_cavallo = {
      -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   5,   5,
      0,   -20, -40, -30, 5,   10,  15,  15,  10,  5,   -30, -30, 0,
      15,  20,  20,  15,  0,   -30, -30, 5,   15,  20,  20,  15,  5,
      -30, -30, 0,   10,  15,  15,  10,  0,   -30, -40, -20, 0,   0,
      0,   0,   -20, -40, -50, -50, -30, -30, -30, -30, -50, -50};

  // tabella di val. pos. TORRE BIANCA
  static constexpr std::array<int, 64> pst_torre = {
      0,  0,  0,  5,  5,  0,  0,  0,  -5, 0, 0, 0, 0, 0, 0, -5,
      -5, 0,  0,  0,  0,  0,  0,  -5, -5, 0, 0, 0, 0, 0, 0, -5,
      -5, 0,  0,  0,  0,  0,  0,  -5, -5, 0, 0, 0, 0, 0, 0, -5,
      5,  10, 10, 10, 10, 10, 10, 5,  0,  0, 0, 0, 0, 0, 0, 0};

  // tabella di val. pos. ALFIERE BIANCO
  static constexpr std::array<int, 64> pst_alfiere = {
      -20, -10, -10, -10, -10, -10, -10, -20, -10, 5,   0,   0,   0,
      0,   5,   -10, -10, 10,  10,  10,  10,  10,  10,  -10, -10, 0,
      10,  10,  10,  10,  0,   -10, -10, 5,   5,   10,  10,  5,   5,
      -10, -10, 0,   5,   10,  10,  5,   0,   -10, -10, 0,   0,   0,
      0,   0,   0,   -10, -20, -10, -40, -10, -10, -40, -10, -20};

  // tabella di val. pos. REGINA BIANCA
  static constexpr std::array<int, 64> pst_regina = {
      -20, -10, -10, -5,  -5,  -10, -10, -20, -10, 0,   5,   0,  0,
      0,   0,   -10, -10, 5,   5,   5,   5,   5,   0,   -10, 0,  0,
      5,   5,   5,   5,   0,   -5,  -5,  0,   5,   5,   5,   5,  0,
      -5,  -10, 0,   5,   5,   5,   5,   0,   -10, -10, 0,   0,  0,
      0,   0,   0,   -10, -20, -10, -10, -5,  -5,  -10, -10, -20};

  // tabella di val. pos. PEDONE BIANCO
  static constexpr std::array<int, 64> pst_pedone = {
      0,  0,  0,  0,   0,   0,  0,  0,  50, 50, 50,  50, 50, 50,  50, 50,
      10, 10, 20, 30,  30,  20, 10, 10, 5,  5,  10,  25, 25, 10,  5,  5,
      0,  0,  0,  20,  20,  0,  0,  0,  5,  -5, -10, 0,  0,  -10, -5, 5,
      5,  10, 10, -20, -20, 10, 10, 5,  0,  0,  0,   0,  0,  0,   0,  0};

  int determina_profondita_ottimale_(
      const Board& b) const;  // calcola la profonditá max

  int valuta_scacchiera_(const Board& b) const;

  int assegna_priorita_mossa_(const movimento& mossa) const;

  void ordina_mosse_(ListaMosse& lista_mosse) const;

  int alfa_beta_(Board& b, const mosse& m, int profondita, int alfa, int beta,
                 int ply) const;  // serve per analizzare gli scenari futuro

 public:
  movimento trova_mossa_migliore(Board& b,
                                 const mosse& m);  // calcola la mossa miglire

  // METODI PUBBLICI USATI SOLO IN test.cpp PER TESTARE I METODI PRIVATI
  int test_valuta_scacchiera(const Board& b) const {
    return valuta_scacchiera_(b);
  }

  int test_determina_profondita_ottimale_(const Board& b) const {
    return determina_profondita_ottimale_(b);
  }
};
}  // namespace scacchi