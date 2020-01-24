#include "Player.hh"


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Private_v1

struct PLAYER_NAME : public Player {

  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }

  /**
   * Types and attributes for your player can be defined here.
   */
  
  typedef vector<int> VInt;         //Vector d'ints
  typedef vector<VInt> MatInt;      //Matriu d'ints
  typedef map<int,int> MapInt;      //Mapa d'ints
  
  typedef vector<bool> VBool;       //Vector de booleans
  typedef vector<VBool> MatBool;    //Matriu de booleans
  
  typedef queue<Pos> QPos;          //Queue de posicions
  typedef vector<Pos> VPos;         //Vector de posicions
  typedef map<Pos,Pos> MapPos;      //Mapa de posicions
  
  struct PosDist {
      Pos p;
      int d;
      PosDist(Pos pos,int dist) : p(pos), d(dist) {}
      bool operator>(const PosDist& altre) const { return d > altre.d; }
  };
  
  typedef priority_queue<PosDist, vector<PosDist>, greater<PosDist>> PQPos;
  
   enum Comp { 
       COMP_CIUTAT_ENEMIGA,     //Busca ciutat enemiga
       COMP_CIUTAT,             //Busca ciutat
       COMP_FUEL,               //Busca fuel
       COMP_ENEMICS             //Busca enemics
  }; 
  
  VPos pAigua;          //Vector de posicions on esta l'aigua
  MatInt MatAigua;      //Matriu de distancies a l'aigua més propera
  
  VPos movimentCotxes;      //Vector de posicions per saber on es moura cada cotxe a cada ronda
  

  /**
   * Play method, invoked once per each round.
   */
/*  
inline bool cotxes_enemics(Pos pos) {   //Serveix per saber si en una area de 7x7(3 de distancia a cada costat) tenint com a centre la posicio del warrior hi ha un cotxe
    for(int i = pos.i -3; i <= pos.i +3; ++i) {
        for (int j = pos.j -3; j <= pos.j +3; ++j) {
            Pos p = Pos(i,j);
            if (pos_ok(p) and cell(p).id != -1 and unit(cell(p).id).type == Car and cell(p).owner != me()) return true;
        }
    }
    return false;
} */ 

inline bool cotxes_enemics(Pos pos) {
    QPos q;
    MatBool visitades(rows(), VBool(cols(), false));
    MapPos pares;
    visitades[pos.i][pos.j] = true;
    q.push(pos);
    
    Pos p;
    
    while (!q.empty()) {
        p = q.front();
        Cell c = cell(p);
        if (c.id != -1 and unit(c.id).type == Car and unit(c.id).player != me()) return true;
        else {
            q.pop();
            VPos nvec;
            for (int d = 0; d != 8; ++d) {
                Dir dir = Dir(d);
                Pos npos = p + dir;
                if (pos_ok(npos) and (npos.i >= pos.i -3) and (npos.i <= pos.i +3) and (npos.j >= pos.j-3) and (npos.j <= pos.j+3)) nvec.push_back(npos);
            }
            for(Pos n : nvec) {
                if (not visitades[n.i][n.j]) {
                    q.push(n);
                    pares[n] = p;
                    visitades[n.i][n.j] = true;
                }
            }
        }
    }
    
    return false;
}
            
inline  bool objectiu(Comp cmp, Cell& c, Unit& u) {     //Serveix perque depenent de quina unitat sigui i del que vulgui fer dir-li quin es el seu objectiu
    if (cmp == COMP_CIUTAT_ENEMIGA) return (c.owner != me() and c.type == City);
    else if (cmp == COMP_CIUTAT) return (c.type == City);
    else if (cmp == COMP_FUEL) return (c.type == Station);
    else if (cmp == COMP_ENEMICS) {
        if (c.id == -1) return false;
        int id_en = unit(c.id).player;
        return id_en != me() and unit(c.id).type != Car;
    }
    else return false;
}

void adjacents(Pos pos, VPos& ret, Unit u) {    //Mira les adjacents a una posició i les agrega al VPos de retorn depenent de certs aspectes
    for (int d = 0; d != 8; ++d) {
        Dir dir = Dir(d);
        Pos npos = pos + dir;
        if (pos_ok(npos)) {
            if (u.type == Warrior) {
                if (cell(npos).type != Wall and cell(npos).type != Water and cell(npos).type != Station ) {
                    if (cell(npos).id != -1) {
                        Unit un = unit(cell(npos).id);
                        if (un.type == Warrior and un.player != me() and un.food < u.food and un.water < u.water) ret.push_back(npos);
                        
                    }
                    else ret.push_back(npos);
                    
                }
                
            }
            else {
                if (cell(npos).type != Wall and cell(npos).type != Water and cell(npos).type != City and cell(npos).type != Station) {
                    if (cell(npos).id != -1) {
                        Unit un = unit(cell(npos).id);
                        if (un.player != me() and un.type != Car) ret.push_back(npos);
                        
                    }
                    else ret.push_back(npos);
                    
                }
            }
        }
    }
}
  
  
VPos bfs_aigua(Pos pos) {       //Retorna un vector amb les posicions de l'aigua
    QPos q; 
    MatBool visitades(rows(), VBool(cols(), false)); 
    MapPos pares; 
    visitades[pos.i][pos.j] = true; 
    q.push(pos);
    
    VPos aux;

    Pos p; 
    while (!q.empty()) { 
      p = q.front(); 
      Cell c = cell(p);
      if (c.type == Water) aux.push_back(p);
      q.pop();
      VPos nvec;
      for (int d = 0; d != 8; ++d) {
          Dir dir = Dir(d);
          Pos npos = p + dir;
          if (pos_ok(npos)) nvec.push_back(npos);
      }
      for (Pos e : nvec) {
          if (not visitades[e.i][e.j]) {
              q.push(e);
              pares[e] = p;
              visitades[e.i][e.j] = true;
          }
        }
      }
    return aux;
}

void dijkstra_aigua(VPos pAigua, MatInt& MatAigua) {        //Retorna una matriu amb les distancies a cada punt d'aigua
    PQPos pq;
    MatInt valors(rows(), VInt(cols(), -1));
    MapPos pares;
    
    for(unsigned int i = 0; i < pAigua.size(); ++i) {
        Pos pos = pAigua[i];
        valors[pos.i][pos.j] = 0;
        pq.push(PosDist(pos,0));
    }
    
    PosDist dp = pq.top();
    
    while (not pq.empty()) {
        dp = pq.top();
        pq.pop();
        
        VPos nvec;
        for (int d = 0; d != 8; ++d) {
          Dir dir = Dir(d);
          Pos npos = dp.p + dir;
          if (pos_ok(npos)) nvec.push_back(npos);
        }
        
        for (Pos e : nvec) {
            int nd = dp.d + 1;
            if (valors[e.i][e.j] == -1 or nd < valors[e.i][e.j]) {
                    pq.push(PosDist(e, nd));
                    valors[e.i][e.j] = nd;
                    pares[e] = dp.p;
                }
            }
    }
    
    MatAigua = valors;
}

Dir vull_aigua(Pos pos) {   //Retorna la posicio més propera per anar a beure aigua 
    
    int value = MatAigua[pos.i][pos.j];
    
    int direccio = value;
    for (int d = 0; d != 8; ++d) {
          Dir dir = Dir(d);
          Pos npos = pos + dir;
          if (pos_ok(npos) and MatAigua[npos.i][npos.j] < value and (cell(npos).id == -1 or ((cell(npos).id != -1) and (unit(cell(npos).id).type == Warrior) and (unit(cell(npos).id).water < unit(cell(pos).id).water) and (unit(cell(npos).id).food < unit(cell(pos).id).food)) )) direccio = d;
    }
    
    return Dir(direccio);
}

Dir bfs(Pos pos, Comp ct, Unit u) {
    QPos q; 
    MatBool visitades(rows(), VBool(cols(), false)); 
    MapPos pares; 
    visitades[pos.i][pos.j] = true; 
    q.push(pos);

    Pos p; 
    bool found = false; 
    while (!q.empty() and not found) { 
      p = q.front(); 
      Cell c = cell(p);
      if (objectiu(ct, c, u)) found = true;
      else {
        q.pop();
        VPos nvec;
        adjacents(p, nvec, u);
        for (Pos n : nvec) {
          if (not visitades[n.i][n.j]) {
            q.push(n);
            pares[n] = p;
            visitades[n.i][n.j] = true;
          }
        }
      }
    }
    while (p != pos and pares[p] != pos) p = pares[p];
    if (p == pos) return Dir(None);

    for (int d = 0; d != 8; ++d) {
      Dir dir = Dir(d);
      Pos npos = pos + dir;
      if (npos == p) {
        return dir;
      }
    }
    return Dir(None);
}

Dir dijkstra(Pos pos, Comp ct, Unit u) {
    PQPos pq;
    MatInt valors(rows(), VInt(cols(), -1));
    MapPos pares;
    valors[pos.i][pos.j];
    pq.push(PosDist(pos,0));
    
    PosDist dp = pq.top();
    
    bool found = false;
    
    while (not found and not pq.empty()) {
        dp = pq.top();
        Cell c = cell(dp.p);
        if (objectiu(ct, c, u)) found = true;
        else {
            pq.pop();
            
            VPos nvec;
            adjacents(dp.p, nvec, u);
            for (Pos n : nvec) {
                int nd = dp.d + 1;
                if (cell(n.i,n.j).type == Desert and ((round()+ nd)%4) != (me()%4)) nd += 4;
                if (cell(n.i,n.j).id != -1 and unit(cell(n.i,n.j).id).type == Car and cell(n.i,n.j).owner != me()) nd += 20;
                
                if (valors[n.i][n.j] == -1 or nd < valors[n.i][n.j]) {
                    pq.push(PosDist(n, nd));
                    valors[n.i][n.j] = nd;
                    pares[n] = dp.p;
                }
            }
        }
    }
    
    Pos p = dp.p;
    
    while (p != pos and pares[p] != pos) p = pares[p];
    
    if (p == pos) return Dir(None);
                  
    for (int d = 0; d != 8; ++d) {
        Dir dir = Dir(d);
        Pos npos = pos + dir;
        if (npos == p) {
            return dir;
        }
    }
    
    return Dir(None);
}
  
    void moure(int id) {
      Unit u = unit(id);
      Dir d = Dir(None);
      
      if (u.type == Warrior) {
          if (cotxes_enemics(u.pos)) d = bfs(u.pos,COMP_CIUTAT,u); //Anar a qualsevol ciutat
          else {
              if (u.water < 15) d = vull_aigua(u.pos);
              else d = bfs(u.pos,COMP_CIUTAT_ENEMIGA,u); //Busca ciutat
          }
      }
      
      else {
          if (u.food == 0) d = bfs(u.pos,COMP_FUEL,u);
          else d = dijkstra(u.pos,COMP_ENEMICS,u);
          
          Pos npos = u.pos + d;
          bool found = false;
          for (unsigned int i = 0; i < movimentCotxes.size() and not found; ++i) {      //Se que pot ser ineficient i molt costos fer una busqueda lineal aquí, pero,
              if (npos == movimentCotxes[i]) found = true;                              //el motiu pel qual ho faig, es que el vector sera de mida, normalment, d'entre
          }                                                                             //3 i 4 ja que molts més cotxes no tindrem, així que el seu cost es petit
          if (found) d = Dir(None);
          else movimentCotxes.push_back(npos);
      }
      
      command(id,d);
      
}
  
  virtual void play () {
      VInt w = warriors(me());
      VInt c = cars(me());
      
      if (round() == 0) {
          
          Pos p (29,29);
          pAigua = bfs_aigua(p);
          
          dijkstra_aigua(pAigua,MatAigua);
          
      }
      
      if (status(me()) > 0.95) return; 
      
      for (int id : w) {
          if (can_move(id)) moure(id);
      }
      
      for (int id : c) {
          if(can_move(id)) moure(id);
      }
      
      movimentCotxes.clear();
  }

};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
