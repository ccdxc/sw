
export enum NodeType {
    workloadSource = 'workloadSource',
    workloadDestination = 'workloadDestination',
    securityGroup = 'securityGroup',
    securityPolicy = 'securityPolicy',
    rule = 'rule',
    naples = 'naples',
    network = 'network'
}

export enum NodeStates {
    loading = 'loading',
    unknown = 'unknown',
    pulsing = 'pulsing',
    error = 'error',
    healthy = 'healthy'
}

export enum LinkStates {
    healthy = 'healthy',
    neutral = 'neutral',
    error = 'error'
}

export enum MarkerTypes {
    arrowHealthy = 'arrow-healthy',
    arrowError = 'arrow-error',
    circleHealthy = 'circle-healthy',
    circleError = 'circle-error',
    None = '',
}

export interface Node {
    type: NodeType;
    state: NodeStates;
    id: string;
    label?: string;
    secondaryLabel?: string;
    labelIcon?: string;
    // position
    x?: number;
    y?: number;
    // Any extra meta data needed
    data?: any;
}

export interface Link {
    sourceID: string;
    targetID: string;
    directed: boolean;
    state: LinkStates;

    // Optional values, will be set by network graph
    // if not provided
    animate?: boolean; /* will use networkgraph's default value if not provided */
    source?: [number, number]; /* will be set by graph using sourceID if not provided */
    target?: [number, number]; /* will be set by graph using targetID if not provided */
    markerStart?: MarkerTypes; /* will be set by graph using the node type and link state */
    markerEnd?: MarkerTypes; /* will be set by graph using the node type and link state */

}
