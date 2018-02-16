import { Action } from '@ngrx/store';
export interface UIAction extends Action {
  type: string;
  payload: any;
}
