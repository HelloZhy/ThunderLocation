function [ t0, tx, ty, tz ] = getDelay( rho, theta, startPoint )
    global gD;
    global gV;

    x = rho * cos(theta * (2 * pi) / 360);
    y = rho * sin(theta * (2 * pi) / 360);
    
    t0 = startPoint;
    tx = (1 / gV) * sqrt((x - gD)^2 + y^2) + t0;
    ty = (1 / gV) * sqrt(x^2 + (y - gD)^2) + t0;
    tz = (1 / gV) * sqrt(x^2 + y^2) + t0;
end

