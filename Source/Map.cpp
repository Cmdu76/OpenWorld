#include "Map.hpp"

namespace owi
{

Map::Map(MapSettings const& settings)
: mSettings(settings)
, mChunks({{ {Chunk(this),Chunk(this),Chunk(this)}
, {Chunk(this),Chunk(this),Chunk(this)}
, {Chunk(this),Chunk(this),Chunk(this)} }})
{
    if (mSettings.generator == nullptr)
    {
        mSettings.generator = new ChunkGenerator();
    }
    mSettings.generator->setMap(this);
    createDirectory(mSettings.directory);
    initChunks();
}

sf::Vector2i Map::getChunkSize() const
{
    return mSettings.chunkSize;
}

sf::Vector2i Map::getTileSize() const
{
    return mSettings.tileSize;
}

sf::Vector2i Map::getTexSize() const
{
    return mSettings.texSize;
}

std::string Map::getDirectory() const
{
    return mSettings.directory;
}

bool Map::isDataCompressed() const
{
    return mSettings.compressedData;
}

Map::Update Map::update(sf::Vector2f position)
{
    auto x = position.x / (mSettings.chunkSize.x * mSettings.tileSize.x);
    auto y = position.y / (mSettings.chunkSize.y * mSettings.tileSize.y * 0.5f);
    sf::Vector2i pos = sf::Vector2i(x,y);
    if (x < 0) pos.x--;
    if (y < 0) pos.y--;

    if (mChunks[1][1].getPos() != pos)
    {
        /*if (mChunks[0][0].getPos() == pos)
        {
            moveTL(pos);
            return Update::TL;
        }
        else if (mChunks[1][0].getPos() == pos)
        {
            moveT(pos);
            return Update::T;
        }
        else if (mChunks[2][0].getPos() == pos)
        {
            moveTR(pos);
            return Update::TR;
        }
        else if (mChunks[0][1].getPos() == pos)
        {
            moveL(pos);
            return Update::L;
        }
        else if (mChunks[2][1].getPos() == pos)
        {
            moveR(pos);
            return Update::R;
        }
        else if (mChunks[0][2].getPos() == pos)
        {
            moveBL(pos);
            return Update::BL;
        }
        else if (mChunks[1][2].getPos() == pos)
        {
            moveB(pos);
            return Update::B;
        }
        else if (mChunks[2][2].getPos() == pos)
        {
            moveBR(pos);
            return Update::BR;
        }
        else
        {
        */

        loadChunks(pos);

        /*
        }
        */
    }
    return Update::Nothing;
}

void Map::render(unsigned int layer, sf::RenderTarget& target) const
{
    #ifdef OWI_INFO
    sf::Clock clock;
    #endif // OWI_INFO

    sf::RenderStates states;
    states.transform *= getTransform();
    unsigned int maxLayer = getMaxLayer();
    sf::Transform layerOffset;
    layerOffset.translate(0,-mSettings.texSize.y+mSettings.tileSize.y);

    for (unsigned int h = 0; h < maxLayer; h++)
    {
        for (unsigned int j = 0; j < static_cast<unsigned int>(3 * mSettings.chunkSize.y); j++)
        {
            for (unsigned int i = 0; i < 3; i++)
            {
                mChunks[i][j/mSettings.chunkSize.y].render(j%mSettings.chunkSize.y,h,target,states);
            }
        }
        states.transform *= layerOffset;
    }

    #ifdef OWI_INFO
    std::cout << "Map: Drawn in : " << clock.restart().asSeconds() << " s" << std::endl;
    #endif // OWI_INFO
}

void Map::render(unsigned int layer, sf::RenderTarget& target, sf::FloatRect viewRect) const
{
    #ifdef OWI_INFO
    sf::Clock clock;
    #endif // OWI_INFO

    sf::RenderStates states;
    states.transform *= getTransform();
    unsigned int maxLayer = getMaxLayer();
    sf::Transform layerOffset;
    layerOffset.translate(0,-mSettings.texSize.y+mSettings.tileSize.y);

    for (unsigned int h = 0; h < maxLayer; h++)
    {
        for (unsigned int j = 0; j < static_cast<unsigned int>(3 * mSettings.chunkSize.y); j++)
        {
            for (unsigned int i = 0; i < 3; i++)
            {
                if (mChunks[i][j/mSettings.chunkSize.y].getBounds().intersects(viewRect))
                {
                    mChunks[i][j/mSettings.chunkSize.y].render(j%mSettings.chunkSize.y,h,target,states,viewRect);
                }
            }
        }
        states.transform *= layerOffset;
    }

    #ifdef OWI_INFO
    std::cout << "Map: Drawn in : " << clock.restart().asSeconds() << " s" << std::endl;
    #endif // OWI_INFO
}

Tileset::Ptr Map::getTileset(std::string const& filename)
{
    if (mTilesets.find(filename) != mTilesets.end())
    {
        return mTilesets[filename];
    }
    else
    {
        loadTileset(filename);
        return mTilesets[filename];
    }
}

bool Map::loadTileset(std::string const& filename)
{
    mTilesets[filename] = std::make_shared<Tileset>(this);
    if (!mTilesets[filename]->loadFromFile(filename))
    {
        #ifdef OWI_DEBUG
        std::cout << "Map: Cant load texture : " << filename << std::endl;
        #endif // OWI_DEBUG
        return false;
    }
    return true;
}

void Map::initChunks(sf::Vector2i pos)
{
    #ifdef OWI_INFO
    sf::Clock clock;
    #endif // OWI_INFO
    for (int j = -1; j < 2; j++)
    {
        for (int i = -1; i < 2; i++)
        {
            if (mSettings.onlineMode && !mSettings.isServer)
            {
                requestChunk(sf::Vector2i(pos.x+i,pos.y+j));
            }
            else
            {
                if (!mChunks[i+1][j+1].loadFromFile(mSettings.directory + std::to_string(pos.x+i) + "_" + std::to_string(pos.y+j) + ".chunk"))
                {
                    mSettings.generator->createChunk(mChunks[i+1][j+1],sf::Vector2i(pos.x+i,pos.y+j));
                }
            }
        }
    }
    #ifdef OWI_INFO
    std::cout << "Map: Chunks initialized in : " << clock.restart().asSeconds() << " s" << std::endl;
    #endif // OWI_INFO
}

void Map::loadChunks(sf::Vector2i pos)
{
    #ifdef OWI_INFO
    sf::Clock clock;
    #endif // OWI_INFO
    for (int j = -1; j < 2; j++)
    {
        for (int i = -1; i < 2; i++)
        {
            if (mSettings.onlineMode && !mSettings.isServer)
            {
                requestChunk(sf::Vector2i(pos.x+i,pos.y+j));
            }
            else
            {
                mChunks[i+1][j+1].saveToFile(mSettings.directory + std::to_string(mChunks[i+1][j+1].getPos().x) + "_" + std::to_string(mChunks[i+1][j+1].getPos().y) + ".chunk");
                if (!mChunks[i+1][j+1].loadFromFile(mSettings.directory + std::to_string(pos.x+i) + "_" + std::to_string(pos.y+j) + ".chunk"))
                {
                    mSettings.generator->createChunk(mChunks[i+1][j+1],sf::Vector2i(pos.x+i,pos.y+j));
                }
            }
        }
    }
    #ifdef OWI_INFO
    std::cout << "Map: Chunks initialized in : " << clock.restart().asSeconds() << " s" << std::endl;
    #endif // OWI_INFO
}

void Map::requestChunk(sf::Vector2i pos)
{

}

void Map::createDirectory(std::string const& filename)
{
    // Need to be cross-platform
    // And only create if the directory dont exist
}

void Map::moveTL(sf::Vector2i pos)
{
    loadChunks(pos);
}

void Map::moveT(sf::Vector2i pos)
{
    loadChunks(pos);
}

void Map::moveTR(sf::Vector2i pos)
{
    loadChunks(pos);
}

void Map::moveL(sf::Vector2i pos)
{
    loadChunks(pos);
}

void Map::moveR(sf::Vector2i pos)
{
    loadChunks(pos);
}

void Map::moveBL(sf::Vector2i pos)
{
    loadChunks(pos);
}

void Map::moveB(sf::Vector2i pos)
{
    loadChunks(pos);
}

void Map::moveBR(sf::Vector2i pos)
{
    loadChunks(pos);
}

unsigned int Map::getMaxLayer() const
{
    unsigned int ret = 0;
    for (unsigned int i = 0; i < 3; i++)
    {
        for (unsigned int j = 0; j < 3; j++)
        {
            ret = std::max(ret,mChunks[i][j].getLayerCount());
        }
    }
    return ret;
}

} // owi
