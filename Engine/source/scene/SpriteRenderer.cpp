/*Later, when the project moves into GameObject + Component architecture, then SpriteRenderer.cpp will matter, because 
instead of hardcoding cats, squares, circles in Renderer, each entity will carry its own SpriteRenderer and call Renderer::texturedQuad
internally.*/