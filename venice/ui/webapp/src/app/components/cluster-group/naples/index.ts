export interface NaplesCondition {
    isHealthy: boolean;
    condition: string;
}

export enum NaplesConditionValues {
    HEALTHY = 'healthy',
    UNHEALTHY = 'unhealthy',
    UNKNOWN = 'unknown',
    EMPTY = '',
}

export enum NodeConditionValues {
    HEALTHY = 'Healthy',
    UNHEALTHY = 'Unhealthy',
    UNKNOWN = 'Unknown'
}
