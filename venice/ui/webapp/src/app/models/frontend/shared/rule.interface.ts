export interface IPRange {
    start: string;
    end: string;
}

export interface PortRange {
    start: number;
    end: number;
}

export interface IPRule {
    ranges?: Array<IPRange>;
    list?: Array<string>;
}

export interface PortRule {
    protocol: string;
    ranges?: Array<PortRange>;
    list?: Array<number>;
}

export enum RuleAction {
    permit = 'permit',
    deny = 'deny'
}

export interface Rule {
    sourceIP: IPRule;
    destIP: IPRule;
    ports: PortRule[];
    action: RuleAction;
}
