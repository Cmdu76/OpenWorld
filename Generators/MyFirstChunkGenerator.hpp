#ifndef MYFIRSTCHUNKGENERATOR_HPP
#define MYFIRSTCHUNKGENERATOR_HPP

#include <cmath>
#include <ctime>

#include "../Source/ChunkGenerator.hpp"

class MyFirstChunkGenerator : public owi::ChunkGenerator
{
    public:
        MyFirstChunkGenerator();

        void createChunk(owi::Chunk& chunk, sf::Vector2i pos);
};

#endif // MYFIRSTCHUNKGENERATOR_HPP