#include "Player.hh"


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Rico


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
  
  enum Comp { COMP_C, COMP_C_M, COMP_E, COMP_A, COMP_F, COMP_E_M}; 
  
  enum wStates {
      WARRIOR_DEFAULT,
      BUSCA_C,
      BUSCA_C_P,
      BUSCA_A,
      BUSCA_E,
      GUARDIAN
  };
  
  enum cStates {
      CAR_DEFAULT,
      MATAR,
      GUARDIA
  };
  
  enum gStates {
      APERCIUTATS,
      ADEFENSARCIUTATS
  };
  
  MapInt sWarriors;
  MapInt sCars;
  
  VPos pAigua;
  MatInt MatAigua;
  
  int gState = APERCIUTATS;
  
  bool cmp_search(Comp cmp, Cell& c, Unit& u) {
    if (cmp == COMP_C) return (c.type == City and c.owner != me());
    else if (cmp == COMP_C_M) return (c.type == City and c.owner == me());
    else if (cmp == COMP_E) {
        if (u.type == Warrior) {
            if (c.id == -1) return false;
            int pid = unit(c.id).player;
            return pid != me() and c.type == City and unit(c.id).food < u.food and unit(c.id).water < u.water;  // No se comprueba me() ya que lo hago en veins()
        }
        else {
            if (c.id == -1) return false;
            int pid = unit(c.id).player;
            return pid != me() and unit(c.id).type != Car;
        }
    }
    else if (cmp == COMP_E_M) {
        if (c.id == -1) return false;
        int pid = unit(c.id).player;
        return pid != me() and c.type == City and c.owner == me() and unit(c.id).food <= u.food and unit(c.id).water <= u.water;
    }
    else if(cmp == COMP_A) return (c.type == Water);
    else if(cmp == COMP_F) return (c.type == Station);

    else return false;
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
                        if (un.player != me()) res.push_back(npos);
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
      if (cmp_search(ct, c, u)) found = true;
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
        if (cmp_search(ct, c, u)) found = true;
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
          if (MatAigua[npos.i][npos.j] < value and pos_ok(npos)) direccio = d;
    }
    
    return Dir(direccio);
}

/*Dir moures_per_la_ciutat(Pos pos, Unit u) {
    
    Dir d = 
    
    VInt c_ciutat;
    for (int d = 0; d != 8; ++d) {
          Dir dir = Dir(d);
          Pos npos = pos + dir;
          Cell c = cell(npos);
          if (c.type == City and c.owner == me() and c.id == -1 and pos_ok(npos)) c_ciutat.push_back(d);
    }
    
    return Dir(c_ciutat[random(0,c_ciutat.size()-1)]);
}*/

void move_w(int id) {
    Unit u = unit(id);
    int wS = sWarriors[id];
    Dir d = Dir(None);
    
    if (wS == WARRIOR_DEFAULT) {
        wS = sWarriors[id] = BUSCA_C;
    }
    
    if (gState == APERCIUTATS) {
        if (u.water == 10) wS = BUSCA_A;
        if (wS == BUSCA_C and cell(u.pos).type == City and cell(u.pos).owner != me()) wS = BUSCA_E;
        
        if (wS == BUSCA_C) d = bfs(u.pos,COMP_C,u);
        if (wS == BUSCA_A) d = vull_aigua(u.pos);
        if (wS == BUSCA_E) d = bfs(u.pos,COMP_E,u);
        if (wS == GUARDIAN) d = bfs(u.pos,COMP_E_M,u);
        
    }
    
    else {
        if (u.water == 10) wS = BUSCA_A;
        if (cell(u.pos).type == City and cell(u.pos).owner == me()) wS = GUARDIAN;
        if (cell(u.pos).type != City) wS = BUSCA_C_P;
        
        if (wS == BUSCA_A) d = vull_aigua(u.pos);
        if (wS == BUSCA_C_P) d = bfs(u.pos,COMP_C_M,u);
        if (wS == GUARDIAN) d = bfs(u.pos,COMP_E_M,u);
    }
    command(id,d);
}

void lookgState() {
    if (num_cities(me()) >= 4) {
        gState = ADEFENSARCIUTATS;
    }
    else gState = APERCIUTATS;
}

  
  /**
   * Play method, invoked once per each round.
   */
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
      
      lookgState();
      
      //if (status(me()) > 0.98) return; //Hem gastat massa CPU
      
      for (int id : w) {
          if (can_move(id)) move_w(id);
      }
      
     
      
      for (int id : c) {
          if(can_move(id)) {
            Pos pos = unit(id).pos;
            Dir dc = dijkstra(pos,COMP_E,unit(id));
            command(id,dc);
          }
      }
  }

};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
