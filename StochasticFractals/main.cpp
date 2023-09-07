#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <utility>
#include <iostream>
#include <time.h>
typedef std::vector<std::vector<sf::Color>> ColorMap;
typedef std::vector<std::pair<sf::Color, double>> ColorRamp;
sf::Vector2i resolution(1024+1, 1024+1);
double roughness = 0.53;
//double roughness = 0.7;

std::mt19937 mersenne(std::time(NULL));
ColorRamp ramp = {{sf::Color::Red, 0},
                  {sf::Color(0xFF5CC5FF), 0.1},
                  {sf::Color::Blue, 0.25},
                  {sf::Color::Green, 0.5},
                  {sf::Color::Yellow, 0.6},
                  {sf::Color(0xFF7F00FF), 0.75}};

sf::Color Mid(std::vector<sf::Color> colors) { 
  int sum_r = 0, sum_g = 0, sum_b = 0;

  for (auto val : colors) {
    sum_r += val.r;
    sum_g += val.g;
    sum_b += val.b;
  }
  return sf::Color(sum_r / colors.size(), sum_g / colors.size(),
                   sum_b / colors.size());
}

double RandomDouble(double min, double max) { 
  return min + (double)mersenne() / mersenne.max() * (max - min);
}

sf::Color RandomColor() { 
  sf::Color res;
  //res.r = mersenne() % 255;
  //res.g = mersenne() % 255;
  //res.b = mersenne() % 255;
  res.r = res.g = res.b = mersenne() % 255;

  return res;
}

sf::Color RandomiseColor(sf::Color color, int i) {
  double min = -std::pow(roughness, i);
  double max = -min;
  int tmp = RandomDouble(min, max)*255;
  //return sf::Color(color.r + RandomDouble(min, max)*255,
  //                 color.g + RandomDouble(min, max)*255,
  //                 color.b + RandomDouble(min, max)*255);
  return sf::Color(color.r + tmp, color.g + tmp, color.b + tmp);
}

void GenerateField(ColorMap& map) {
  int square_size = (resolution.x - 1);
  int depth = 1;

  while (square_size != 1) {
    for (int i = square_size / 2; i < resolution.y; i += square_size) {
      for (int j = square_size / 2; j < resolution.x; j += square_size) {
        auto top_left_c = map[i - square_size / 2][j - square_size / 2];
        auto top_right_c = map[i - square_size / 2][j + square_size / 2];
        auto down_left_c = map[i + square_size / 2][j - square_size / 2];
        auto down_right_c = map[i + square_size / 2][j + square_size / 2];

        map[i][j] = RandomiseColor(
            Mid({top_left_c, top_right_c, down_left_c, down_right_c}), depth);
      }
    }

    bool even = true;
    for (int i = 0; i < resolution.y; i += square_size / 2) {
      for (int j = (even) ? square_size / 2 : 0; j < resolution.x;
           j += square_size) {
        auto top = map[(resolution.y + i - square_size / 2) % resolution.y][j];
        auto left = map[i][(resolution.x + j - square_size / 2) % resolution.x];
        auto right = map[i][(j + square_size / 2) % resolution.x];
        auto down = map[(i + square_size / 2) % resolution.y][j];

        map[i][j] = RandomiseColor(Mid({left, right, top, down}), depth);
      }
      even = !even;
    }

    depth++;
    square_size /= 2;
  }
}

sf::Color LerpColor(sf::Color a, sf::Color b, double alpha) {
  sf::Color res;
  res.r = a.r + (b.r - a.r) * alpha;
  res.g = a.g + (b.g - a.g) * alpha;
  res.b = a.b + (b.b - a.b) * alpha;
  res.a = a.a + (b.a - a.a) * alpha;
  return res;
}

sf::Color GetColorFromRamp(ColorRamp& ramp, double point) {
  for (int i = 0; i < ramp.size() - 1; ++i) {
    if (point >= ramp[i].second && point <= ramp[i + 1].second) {
      point = (point - ramp[i].second) / (ramp[i + 1].second - ramp[i].second);
      return LerpColor(ramp[i].first, ramp[i + 1].first, point);
    }
  }
  if (point < ramp[0].second) {
    point = 0;
    return LerpColor(ramp[0].first, ramp[1].first, point);
  }
  if (point > ramp.back().second) {
    point = 1;
    return LerpColor(ramp[ramp.size() - 2].first, ramp.back().first, point);
  }
}

int main() {
  sf::RenderWindow window(sf::VideoMode(resolution.x, resolution.y),
                          "Stochastic Fractal");
  window.clear(sf::Color::White);
  window.setVerticalSyncEnabled(true);
  window.setActive(true);

  ColorMap map(resolution.y, std::vector<sf::Color>(resolution.x));
  map[0][0] = RandomColor();
  map[0][resolution.x - 1] = RandomColor();
  map[resolution.y - 1][0] = RandomColor();
  map[resolution.y - 1][resolution.x - 1] = RandomColor();
  std::cout << "bebra";
  GenerateField(map);
  sf::VertexArray vertices(sf::Points);
  for (int i = 0; i < resolution.y; ++i) {
    for (int j = 0; j < resolution.x; ++j) {
      map[i][j] =
          GetColorFromRamp(ramp, std::pow((double)map[i][j].r / 255, 2));
      vertices.append(sf::Vertex(sf::Vector2f(j, i), map[i][j]));
    }
  }

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }
      else if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::R) {
          GenerateField(map);
          vertices.clear();
          for (int i = 0; i < resolution.y; ++i) {
            for (int j = 0; j < resolution.x; ++j) {
              map[i][j] = GetColorFromRamp(
                  ramp, std::pow((double)map[i][j].r / 255, 2));
              vertices.append(sf::Vertex(sf::Vector2f(j, i), map[i][j]));
            }
          }
        }
      }
    }

    window.clear(sf::Color::White);
    window.draw(vertices);
    window.display();
  }
}