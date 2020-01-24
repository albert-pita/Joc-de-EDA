#include "Player.hh"


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Skipper


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
      COMP_C,   //Busca ciutat
      COMP_C_M, //Busca ciutat propia
      COMP_E,   //Busca enemics
      COMP_F,   //Busca fuel
      COMP_E_SUICIDA
  }; 
  
  enum wStates {
      WARRIOR_DEFAULT,
      BUSCA_C,
      GUARDIAN,
      BUSCA_A
  };
  
  enum cStates {
      CAR_DEFAULT,
      MATAR,
      SUICIDA,
      BUSCA_F
  };
  
  enum gStates {
      GAME_DEFAULT,
      CONQUERIR,
      DEFENSAR
  };
  
  const VInt pDEFENSES = { BUSCA_C, GUARDIAN };
  
  const VInt* pDEFENSAR = &pDEFENSES;
  
  const VInt pWGAME_DEFAULT = { BUSCA_C, BUSCA_C, BUSCA_C, BUSCA_C, BUSCA_C,
                               BUSCA_C, BUSCA_C, GUARDIAN, GUARDIAN, GUARDIAN};
                               
  const VInt pWCONQUERIR = { BUSCA_C, BUSCA_C, BUSCA_C, BUSCA_C, BUSCA_C,
                            BUSCA_C, BUSCA_C, BUSCA_C, BUSCA_C, GUARDIAN};
                            
  const VInt pWDEFENSAR = { GUARDIAN, GUARDIAN, GUARDIAN, GUARDIAN, GUARDIAN,
                           GUARDIAN, GUARDIAN, GUARDIAN, GUARDIAN, BUSCA_C};
                           
  const VInt* pWActuals = &pWGAME_DEFAULT;
  
  const VInt* pWGame(int sGame) {
      if (sGame == GAME_DEFAULT) return &pWGAME_DEFAULT;
      if (sGame == CONQUERIR) return &pWCONQUERIR;
      if (sGame == DEFENSAR) return &pWDEFENSAR;
      return &pWGAME_DEFAULT;
  }
  
  const VInt pCGAME_DEFAULT = { MATAR, MATAR, MATAR, MATAR, MATAR,
                                MATAR, MATAR, SUICIDA, SUICIDA, SUICIDA};
                                
  const VInt pCCONQUERIR = { MATAR, MATAR, MATAR, MATAR, MATAR,
                             MATAR, MATAR, MATAR, MATAR, SUICIDA};
                             
  const VInt pCDEFENSAR = { SUICIDA, SUICIDA, SUICIDA, SUICIDA, SUICIDA,
                            SUICIDA, SUICIDA, SUICIDA, SUICIDA, MATAR};
                            
  const VInt* pCActuals = &pCGAME_DEFAULT;
                            
  const VInt* pCGame(int sGame) {
      if (sGame == GAME_DEFAULT) return &pCGAME_DEFAULT;
      if (sGame == CONQUERIR) return &pCCONQUERIR;
      if (sGame == DEFENSAR) return &pCDEFENSAR;
      return &pCGAME_DEFAULT;
  }
  
  MapInt sWarriors;
  MapInt sCars;
  
  VPos pAigua;
  MatInt MatAigua;
  
  int gState = GAME_DEFAULT;
  /**
   * Play method, invoked once per each round.
   */
  
inline bool no_companys(Pos pos) {
      for (int i = pos.i -2; i <= pos.i +2; ++i) {
          for (int j = pos.j -2; j <= pos.j +2; ++j) {
              Pos p (i,j);
              if (pos_ok(p) and cell(p).type == City and cell(p).id != -1 and cell(p).owner == me() and cell(p).id != cell(pos).id) return false;
          }
      }
      return true;
} 
                  
  
inline int percentatgeWarriors() {
      int total = nb_players() * nb_warriors();
      
      int myW = warriors(me()).size();
      return (100 * myW) / total;
}
  
inline  bool guanyant() {
      int max = 0;
      for (int j = 0; j < nb_players(); ++j) {
          int res = total_score(j);
          if (res > max) max = res;
      }
      
      return max == total_score(me());
}
  
inline  bool Objectiu(Comp cmp, Cell& c, Unit& u) {
    if (cmp == COMP_C) return (c.type == City and c.owner != me());
    else if (cmp == COMP_C_M) return (c.type == City and c.owner == me());
    else if (cmp == COMP_E) {
        if (u.type == Car) {
            if (c.id == -1) return false;
            int pid = unit(c.id).player;
            return pid != me() and unit(c.id).type != Car;
        }
        else {
            if (c.id == -1) return false;
            int pid = unit(c.id).player;
            return pid != me() and unit(c.id).water < u.water and unit(c.id).food < u.food;
        }
    }
    else if (cmp == COMP_E_SUICIDA) {
        if (c.id == -1) return false;
        int pid = unit(c.id).player;
        return pid != me();
    }
    else if(cmp == COMP_F) return (c.type == Station);

    else return false;
}
  
  void revalorar () {
      if (round() % 500 == 0) {
          int lastgState = gState;
          
          switch (gState) {
              case CONQUERIR:
                  if (guanyant()) gState = GAME_DEFAULT;
                  if (percentatgeWarriors() > 1.1 * 100/nb_players() and guanyant()) gState = DEFENSAR;
                  break;
              case DEFENSAR:
                  if (percentatgeWarriors() > 1.1 * (100 / nb_players())) gState = CONQUERIR;
                  break;
              case GAME_DEFAULT:
              default:
                  if (not guanyant()) gState = CONQUERIR;
                  //if (percentatgeWarriors() < 100/nb_players()) gState = DEFENSAR;
                  break;
          }
          
          pWActuals = pWGame(gState);
          pCActuals = pCGame(gState);
          
          if (gState != lastgState) {
              sWarriors.clear();
              sCars.clear();
          }
      }
}
  
  void moure(int id) {
    Unit u = unit(id);
    Dir d = Dir(None);
    
    if (u.type == Warrior) {
        int wS = sWarriors[id];
        
        if (wS == WARRIOR_DEFAULT) {                            //Aixo esta bé
            wS = sWarriors[id] = (*pWActuals)[random(0, 9)];
        }
        
        if (u.water == 10) {
            wS = BUSCA_A;
            sWarriors[id] = BUSCA_A;
        }
        if (wS == BUSCA_A and u.water > 30) {
            wS = BUSCA_C;
            sWarriors[id] = BUSCA_C;
        }
        
        if (round() > 4 and wS == BUSCA_C and cell(u.pos).type == City) {
            wS = sWarriors[id] = (*pDEFENSAR)[random(0,1)];
        }
        
        if (wS == BUSCA_C) d = bfs(u.pos,COMP_C,u);
        if (wS == BUSCA_A) d = vull_aigua(u.pos);
        if (wS == GUARDIAN) {
            if (cell(u.pos).type == City and cell(u.pos).owner == me()) d = bfs_ciutat(u.pos,COMP_E,u); //Potser crear un bfs 
            else if (cell(u.pos).type == City and cell(u.pos).owner != me()) d = bfs_ciutat(u.pos,COMP_E,u);
            else d = bfs(u.pos,COMP_C_M,u);
        }
    }
    
    else {
        int cS = sCars[id];
        
        if (cS == CAR_DEFAULT) {
            cS = sCars[id] = (*pCActuals)[random(0, 9)];
        }
        
        if (u.food < 10) {
            cS = BUSCA_F;
            sCars[id] = BUSCA_F;
        }
        
        if (cS == BUSCA_F and u.food > 90) {
            cS = MATAR;
            sCars[id] = MATAR;
        }
        
        if (cS == MATAR) d = dijkstra(u.pos,COMP_E,u);
        if (cS == SUICIDA) d = dijkstra(u.pos,COMP_E_SUICIDA,u);
        if (cS == BUSCA_F) d = bfs(u.pos,COMP_F,u);
    }
            
        
    command(id,d);
}

void veins(Pos pos, VPos& res, Unit myUnit) {
    for (int d = 0; d != 8; ++d) {
      Dir dir = Dir(d);
      Pos npos = pos + dir;
      if (pos_ok(npos)) {
            if (myUnit.type == Warrior) {
                if (cell(npos).type != Wall and cell(npos).type != Water and cell(npos).type != Station ) {
                    if (cell(npos).id != -1) {
                        Unit un = unit(cell(npos).id);
                        if (un.type == Warrior and un.player != me() and un.food < myUnit.food and un.water < myUnit.water) res.push_back(npos);
                }
                else res.push_back(npos);
            }
        }
            else {
                if (cell(npos).type != Wall and cell(npos).type != Water and cell(npos).type != City and cell(npos).type != Station) {
                    if (cell(npos).id != -1) {
                        Unit un = unit(cell(npos).id);
                        if (un.player != me() and un.type != Car) res.push_back(npos);
                    }
                else res.push_back(npos);
            }
            }
      }
          
    }
}

Dir bfs(Pos pos, Comp ct, Unit u) {
    QPos q; 
    MatBool visited(rows(), VBool(cols(), false)); 
    MapPos parents; 
    visited[pos.i][pos.j] = true; 
    q.push(pos);

    Pos p; 
    bool found = false; 
    while (!q.empty() and not found) { 
      p = q.front(); 
      Cell c = cell(p);
      if (Objectiu(ct, c, u)) found = true;
      else {
        q.pop();
        VPos neig;
        veins(p, neig, u);
        for (Pos n : neig) {
          if (not visited[n.i][n.j]) {
            q.push(n);
            parents[n] = p;
            visited[n.i][n.j] = true;
          }
        }
      }
    }
    while (p != pos and parents[p] != pos) p = parents[p];
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
    MatInt prices(rows(), VInt(cols(), -1));
    MapPos parents;
    prices[pos.i][pos.j];
    pq.push(PosDist(pos,0));
    
    PosDist dp = pq.top();
    
    bool found = false;
    
    while (not found and not pq.empty()) {
        dp = pq.top();
        Cell c = cell(dp.p);
        if (Objectiu(ct, c, u)) found = true;
        else {
            pq.pop();
            
            VPos neig;
            veins(dp.p, neig, u);
            for (Pos n : neig) {
                int newd = dp.d + 1;
                if (cell(n.i,n.j).type == Desert and ((round()+ newd)%4) != (me()%4)) newd += 4;
                
                if (prices[n.i][n.j] == -1 or newd < prices[n.i][n.j]) {
                    pq.push(PosDist(n, newd));
                    prices[n.i][n.j] = newd;
                    parents[n] = dp.p;
                }
            }
        }
    }
    
    Pos p = dp.p;
    
    while (p != pos and parents[p] != pos) p = parents[p];
    
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

VPos bfs_aigua(Pos pos) {
    QPos q; 
    MatBool visited(rows(), VBool(cols(), false)); 
    MapPos parents; 
    visited[pos.i][pos.j] = true; 
    q.push(pos);
    
    VPos aux;

    Pos p; 
    while (!q.empty()) { 
      p = q.front(); 
      Cell c = cell(p);
      if (c.type == Water) aux.push_back(p);
      q.pop();
      VPos neig;
      for (int d = 0; d != 8; ++d) {
          Dir dir = Dir(d);
          Pos npos = p + dir;
          if (pos_ok(npos)) neig.push_back(npos);
      }
      for (Pos n : neig) {
          if (not visited[n.i][n.j]) {
              q.push(n);
              parents[n] = p;
              visited[n.i][n.j] = true;
          }
        }
      }
    return aux;
}

void dijkstra_aigua(VPos pAigua, MatInt& MatAigua) {
    PQPos pq;
    MatInt prices(rows(), VInt(cols(), -1));
    MapPos parents;
    
    for(unsigned int i = 0; i < pAigua.size(); ++i) {
        Pos pos = pAigua[i];
        prices[pos.i][pos.j] = 0;
        pq.push(PosDist(pos,0));
    }
    
    PosDist dp = pq.top();
    
    while (not pq.empty()) {
        dp = pq.top();
        pq.pop();
        
        VPos neig;
        for (int d = 0; d != 8; ++d) {
          Dir dir = Dir(d);
          Pos npos = dp.p + dir;
          if (pos_ok(npos)) neig.push_back(npos);
        }
        
        for (Pos n : neig) {
            int newd = dp.d + 1;
            if (prices[n.i][n.j] == -1 or newd < prices[n.i][n.j]) {
                    pq.push(PosDist(n, newd));
                    prices[n.i][n.j] = newd;
                    parents[n] = dp.p;
                }
            }
    }
    
    MatAigua = prices;
}

Dir vull_aigua(Pos pos) {   
    
    int value = MatAigua[pos.i][pos.j];
    
    int direccio = value;
    for (int d = 0; d != 8; ++d) {
          Dir dir = Dir(d);
          Pos npos = pos + dir;
          if (MatAigua[npos.i][npos.j] < value and (cell(npos).id == -1 or ((cell(npos).id != -1) and (unit(cell(npos).id).type == Warrior) and (unit(cell(npos).id).water < unit(cell(pos).id).water) and (unit(cell(npos).id).food < unit(cell(pos).id).food)) and pos_ok(npos))) direccio = d;
    }
    
    return Dir(direccio);
}

/*Dir moures_per_la_ciutat(Pos pos, Comp ct, Unit u) {
    
    Dir d = Dir(None);
    if (no_companys(pos)) {
        
        VInt c_ciutat;
        for (int d = 0; d != 8; ++d) {
            Dir dir = Dir(d);
            Pos npos = pos + dir;
            Cell c = cell(npos);
            if (ct == COMP_C) {
                if (c.type == City and c.owner == me() and pos_ok(npos)) {
                    if (c.id != -1) {
                        Unit un = unit(c.id);
                        if (un.type == Warrior and un.player != me() and un.food < u.food and un.water < u.water) return Dir(d);
                }
                else c_ciutat.push_back(d);
                }
            }
            else {
                if (c.type == City and c.owner != me() and pos_ok(npos)) {
                    if (c.id != -1) {
                        Unit un = unit(c.id);
                        if (un.type == Warrior and un.player != me() and un.food < u.food and un.water < u.water) return Dir(d);
                }
                else c_ciutat.push_back(d);
                }
            }
        }
        d = Dir(c_ciutat[random(0,c_ciutat.size()-1)]);
    }
    
    return d;
}*/

Dir bfs_ciutat(Pos pos, Comp ct, Unit u) {
    QPos q; 
    MatBool visited(rows(), VBool(cols(), false)); 
    MapPos parents; 
    visited[pos.i][pos.j] = true; 
    q.push(pos);

    Pos p; 
    bool found = false; 
    while (!q.empty() and not found) { 
      p = q.front(); 
      Cell c = cell(p);
      if (Objectiu(ct, c, u)) found = true;
      
      
      else {
        q.pop();
        VPos neig;
        for (int d = 0; d != 8; ++d) {
          Dir dir = Dir(d);
          Pos npos = p + dir;
          if (pos_ok(npos) and cell(npos).type == City) neig.push_back(npos);
        }
        for (Pos n : neig) {
          if (not visited[n.i][n.j]) {
            q.push(n);
            parents[n] = p;
            visited[n.i][n.j] = true;
          }
        }
      }
    }
    while (p != pos and parents[p] != pos) p = parents[p];
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
  
  
  
  virtual void play () {
      
      VInt w = warriors(me());
      VInt c = cars(me());
      
      if (round() == 0) {
          
          Pos p (29,29);
          pAigua = bfs_aigua(p);
          
          dijkstra_aigua(pAigua,MatAigua);
          
          VInt rPermutation = random_permutation(w.size());
          unsigned int count = 0;
          for (int i : rPermutation) {
              if (count < (w.size()/4)) sWarriors[w[i]] = GUARDIAN;
              else sWarriors[w[i]] = BUSCA_C;
              ++count;
          }
        
          for (int i : c) sCars[c[i]] = MATAR;
              
      }
      
      revalorar();
      
      if (status(me()) > 0.95) return; 
      
      for (int id : w) {
          if (can_move(id)) moure(id);
      }
      
      for (int id : c) {
          if(can_move(id)) moure(id);
      }
  }

};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
