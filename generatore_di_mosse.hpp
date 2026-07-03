#pragma once
#include <algorithm>
#include <array>

namespace scacchi {
template <size_t N>

class ArrayMosse {
  std::array<int, N> dati_;
  size_t conteggio_{0};

 public:
  void push_back(int val) {
    dati_[conteggio_] = val;
    ++conteggio_;
  }

  size_t size() const { return conteggio_; }
  bool empty() const { return conteggio_ == 0; }
  int operator[](size_t i) const { return dati_[i]; }
  auto begin() const { return dati_.begin(); }
  auto end() const { return dati_.begin() + conteggio_; }
  auto begin() { return dati_.begin(); }
  auto end() { return dati_.begin() + conteggio_; }
};

struct mosse_pedone {
  ArrayMosse<2> mossa_normale;
  ArrayMosse<2> cattura_del_pedone;
};

struct mosse_direzioni_torre {
  std::array<ArrayMosse<7>, 4> direzioni;
};

struct mosse_direzioni_alfiere {
  std::array<ArrayMosse<7>, 4> direzioni;
};

struct mosse_direzioni_regina {
  std::array<ArrayMosse<7>, 8> direzioni;
};

struct mosse {
  std::array<ArrayMosse<8>, 64> mosse_cavallo;
  std::array<ArrayMosse<8>, 64> mosse_re;
  std::array<mosse_direzioni_torre, 64> mosse_torre;
  std::array<mosse_direzioni_alfiere, 64> mosse_alfiere;
  std::array<mosse_direzioni_regina, 64> mosse_regina;
  std::array<mosse_pedone, 64> mosse_pedone_bianco;
  std::array<mosse_pedone, 64> mosse_pedone_nero;
  mosse();

    // ❌ VIETA LA COPIA PER EVITARE RALLENTAMENTI ACCIDENTALI
    mosse(const mosse&) = delete;            // Costruttore di copia vietato
    mosse& operator=(const mosse&) = delete; // Operatore di assegnazione per copia vietato

    //   CONSENTI LO SPOSTAMENTO (MOVE SEMANTICS)
    mosse(mosse&&) noexcept = default;            // Costruttore di spostamento
    mosse& operator=(mosse&&) noexcept = default;
};

bool e_dentro_scacchiera(int x, int y);
// Dichiarazioni delle funzioni calcolatrici
void calcolatore_mosse_RE(mosse& m);
void calcolatore_mosse_CAVALLO(mosse& m);
void calcolatore_mosse_TORRE_e_REGINA(mosse& m);
void calcolatore_mosse_ALFIERE_e_REGINA(mosse& m);
void calcolatore_mosse_pedone_nero(mosse& m);
void calcolatore_mosse_pedone_bianco(mosse& m);

}  // namespace scacchi