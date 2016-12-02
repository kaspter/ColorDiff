






// finds a cosine of angle between vectors from pt0->pt1 and from pt0->pt2
double angle( Point pt1, Point pt2, Point pt0 ) {
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    double ratio;//边长平方的比

    ratio=(dx1*dx1+dy1*dy1)/(dx2*dx2+dy2*dy2);
    if (ratio<0.8 || 1.2<ratio) {//根据边长平方的比过小或过大提前淘汰这个四边形，如果淘汰过多，调整此比例数字
        return 1.0;//根据边长平方的比过小或过大提前淘汰这个四边形
    }
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}
