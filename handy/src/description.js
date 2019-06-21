class Point {
  constructor(r, c){
    this.r = r;
    this.c = c;
  }
};

class Description {
  constructor(text){
    const parse_point = function(s){
      const re = /^\((\d+),(\d+)\)$/;
      const found = s.match(re);
      if(found === null){ throw new Error("invalid point: " + s); }
      return new Point(parseInt(found[2]), parseInt(found[1]));
    };
    const parse_map = function(s){
      const re = /\(\d+,\d+\)/g;
      const matches = s.match(re);
      if(matches === null){ return []; }
      return s.match(re).map(parse_point);
    };
    const parse_obstacles = function(s){
      return s.split(';').map(parse_map);
    };
    const parse_task = function(s){
      const blocks = s.split('#');
      return {
        "map": parse_map(blocks[0]),
        "point": parse_point(blocks[1]),
        "obstacles": parse_obstacles(blocks[2]),
        "boosters": null
      };
    };
    const direction = function(a, b){
      if(a.r == b.r){ return a.c < b.c ? 0 : 2; }
      return a.r < b.r ? 1 : 3;
    };
    const task = parse_task(text);
    const H = Math.max(...(task.map.map(p => p.r)));
    const W = Math.max(...(task.map.map(p => p.c)));
    let differential = Array.from(new Array(H + 1), () => new Array(W + 1).fill(0));
    const update_differential = (poly) => {
      for(let i = 0; i < poly.length; ++i){
        const a = poly[i];
        const b = poly[(i + 1) % poly.length];
        const d = direction(a, b);
        if(d == 1){
          for(let k = a.r; k < b.r; ++k){ differential[k][a.c] ^= 1; }
        }else if(d == 3){
          for(let k = b.r; k < a.r; ++k){ differential[k][a.c] ^= 1; }
        }
      }
    };
    update_differential(task.map);
    task.obstacles.forEach(update_differential);
    let grid = Array.from(new Array(H), () => new Array(W).fill(0));
    for(let i = 0; i < H; ++i){
      let state = 0;
      for(let j = 0; j < W; ++j){
        if(differential[i][j]){ state ^= 1; }
        grid[i][j] = (state ? CELL_EMPTY : CELL_OBSTACLE);
      }
    }
    this.grid = grid;
    this.point = task.point;
  }
}
