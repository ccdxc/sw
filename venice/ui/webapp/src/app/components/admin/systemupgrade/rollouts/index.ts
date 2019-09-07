import { SelectItem } from 'primeng/primeng';


export interface RolloutImageLabel {
    Description: string;
    Environment?: string;
    Releasedate?: string;
    Version: string;
}

export enum EnumRolloutOptions {
    'naplesonly' = 'DSC Only',
    'veniceonly' = 'Venice Only',
    'both' = 'Both DSC and Venice'
}

export interface RolloutImageOption extends SelectItem {
    model?: RolloutImageLabel;
}
