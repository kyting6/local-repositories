#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include<algorithm>
#include <sstream>
#define INFINITY 1e+9
struct Point {
    int x, y;
    Point() : Point(0, 0) {}
	Point(int x, int y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};
int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;
class OthelloBoard {
public:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;
private:
    int get_next_player(int player) const {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
public:

    OthelloBoard()
    {
        reset();
    }
    OthelloBoard(std::array<std::array<int,SIZE>,SIZE> b,std::vector<Point> n,int p)
    {
        int black=0,white=0;
        for(int i=0;i<8;i++)
        {
            for(int j=0;j<8;j++)
            {
                if(board[i][j]==1) black++;
                else white++;
            }
        }
        board=b;
        cur_player=p;
        done = false;
        winner = -1;
        disc_count[EMPTY] = 64 - black - white;
        disc_count[BLACK] = black;
        disc_count[WHITE] = white;
        next_valid_spots=get_valid_spots();
    }
    void reset() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }
        board[3][4] = board[4][3] = BLACK;
        board[3][3] = board[4][4] = WHITE;
        cur_player = BLACK;
        disc_count[EMPTY] = 8*8-4;
        disc_count[BLACK] = 2;
        disc_count[WHITE] = 2;
        next_valid_spots = get_valid_spots();
        done = false;
        winner = -1;
    }
    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    bool put_disc(Point p) {
        if(!is_spot_valid(p)) {
            winner = get_next_player(cur_player);
            done = true;
            return false;
        }
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        // Check Win
        if (next_valid_spots.size() == 0) {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            if (next_valid_spots.size() == 0) {
                // Game ends
                done = true;
                int white_discs = disc_count[WHITE];
                int black_discs = disc_count[BLACK];
                if (white_discs == black_discs) winner = EMPTY;
                else if (black_discs > white_discs) winner = BLACK;
                else winner = WHITE;
            }
        }
        return true;
    }
};
void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}
bool is_corner(int x,int y)
{
    if((x==0&&y==0)||(x==0&&y==7)||(x==7&&y==0)||(x==7&&y==7)) return true;
    else return false;
}
bool is_cornerside(int x,int y)
{
    if((x==1&&y==0)||(x==0&&y==1)||(x==6&&y==0)||(x==7&&y==1)||(x==0&&y==6)||(x==1&&y==7)||(x==6&&y==7)||(x==7&&y==6)) return true;
    else return false;
}
bool is_safex(OthelloBoard s,int x,int y)
{
    if(s.board[0][0]==s.board[x][y]&&x==1&&y==1) return true;
    else if(s.board[7][0]==s.board[x][y]&&x==6&&y==1) return true;
    else if(s.board[0][7]==s.board[x][y]&&x==1&&y==6) return true;
    else if(s.board[7][7]==s.board[x][y]&&x==6&&y==6) return true;
    else return false;
}
bool is_safeside(OthelloBoard s,int x,int y)
{
     if(s.board[0][0]==s.board[x][y]&& ( (x==1&&y==0) ||(x==0&&y==1) )) return true;
    else if(s.board[7][0]==s.board[x][y]&& ( (x==6&&y==0) ||(x==7&&y==1) )) return true;
    else if(s.board[0][7]==s.board[x][y]&& ( (x==1&&y==7) ||(x==0&&y==6) )) return true;
    else if(s.board[7][7]==s.board[x][y]&& ( (x==6&&y==7) ||(x==7&&y==6) )) return true;
    else return false;
}
bool is_bound(int x,int y)
{
    if(is_corner(x,y)||is_cornerside(x,y)) return false;
    if(x==0||x==7||y==0||y==7) return true;
    else return false;
}
bool is_x(int x,int y)
{
    if((x==1&&y==1)||(x==1&&y==6)||(x==6&&y==1)||(x==6&&y==6)) return true;
    else return false;
}
int evaluate(OthelloBoard s)
{
    int val=0;
    int state_value[8][8]={
    65,  -3, 6, 4, 4, 6, -3,  65,
    -3, -29, 3, 1, 1, 3, -29, -3,
     6,   3, 5, 3, 3, 5,  3,   6,
     4,   1, 3, 1, 1, 3,  1,   4, 
     4,   1, 3, 1, 1, 3,  1,   4,
     6,   3, 5, 3, 3, 5,  3,   6, 
    -3, -29, 3, 1, 1, 3, -29, -3,  
    65,  -3, 6, 4, 4, 6,  -3, 65,
    };
    for(int i=0;i<8;i++)
    {
        for(int j=0;j<8;j++)
        {
            if(s.board[i][j]==player)
            {
                if(is_corner(i,j)) val+=65;
                else if(is_x(i,j)&&is_safex(s,i,j)) val+=0;
                else if(is_x(i,j)&& !is_safex(s,i,j)) val-=39;  
                else if(is_cornerside(i,j)&&is_safeside(s,i,j)) val+=0;
                else if(is_cornerside(i,j)&& !is_safeside(s,i,j)) val-=20;
                else if(is_bound(i,j)) val+=5;
                else val+=state_value[i][j];
            }
            else if(s.board[i][j]==3-player)
            {
                if(is_corner(i,j)) val-=100;
                else if(is_x(i,j)&&is_safex(s,i,j)) val-=1;
                else if(is_x(i,j)&& !is_safex(s,i,j)) val+=29;  
                else if(is_cornerside(i,j)&&is_safeside(s,i,j)) val-=3;
                else if(is_cornerside(i,j)&& !is_safeside(s,i,j)) val+=20;
                else if(is_bound(i,j)) val-=7;
                else val-=state_value[i][j];
            }
        }
    }
    if(s.cur_player==3-player) val-=s.next_valid_spots.size()*4;
    else if(s.cur_player==player) val+=s.next_valid_spots.size()*2;
    return val;
}
int minimax(OthelloBoard s,int depth,int alpha,int beta,int p)
{
    //s.next_valid_spots=s.get_valid_spots();
    if(depth==0 || next_valid_spots.empty()) return evaluate(s);
    if(p==player)
    {
        int value=(-1)*INFINITY;
        for(int i=0;i<s.next_valid_spots.size();i++)
        {
            OthelloBoard child(s.board,s.next_valid_spots,s.cur_player);
            child.put_disc(s.next_valid_spots[i]);
            value=std::max(value,minimax(child,depth-1,alpha,beta,3-p));
            alpha=std::max(alpha,value);
            if(alpha>=beta)
            {
                break;
            }
        }
        return value;
    }
    else
    {
        int value=INFINITY;
        for(int i=0;i<s.next_valid_spots.size();i++)
        {
            OthelloBoard child(s.board,s.next_valid_spots,s.cur_player);
            child.put_disc(s.next_valid_spots[i]);
            value=std::min(value,minimax(child,depth-1,alpha,beta,3-p));
            beta=std::min(beta,value);
            if(beta<=alpha) break;
        }
        return value;
    }
}

void write_valid_spot(std::ofstream& fout) {
    int n_valid_spots = next_valid_spots.size();
    srand(time(NULL));
    // Choose random spot. (Not random uniform here)
    int index = 0;
    int alpha=-INFINITY,beta=INFINITY;
    for(int i =0;i<n_valid_spots;i++)
    {
        OthelloBoard next(board,next_valid_spots,player);
        next.put_disc(next_valid_spots[i]);
        int value=0;
        if(next.disc_count[0]==13) value=minimax(next,13,alpha,beta,3-player);
        else if(next.disc_count[0]==11)value=minimax(next,11,alpha,beta,3-player);
         else if(next.disc_count[0]==9)value=minimax(next,9,alpha,beta,3-player);
          else if(next.disc_count[0]==7)value=minimax(next,7,alpha,beta,3-player);
           else value=minimax(next,5,alpha,beta,3-player);
        if(value>alpha) alpha=value,index=i;
    }
    Point p = next_valid_spots[index];
    // Remember to flush the output to ensure the last action is written to file.
    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
