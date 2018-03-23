interface Margin {
    top?: string;
    left?: string;
    right?: string;
    bottom?: string;
}

export interface Icon {
    margin: Margin;

    /* Name of the mat icon to load */
    matIcon?: string;

    /* Name of the svgIcon to load */
    svgIcon?: string;
}
