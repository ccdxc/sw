import { Injectable } from '@angular/core';
import { Actions, Effect } from '@ngrx/effects';
import { Observable } from 'rxjs/Observable';
import { tap } from 'rxjs/operators/tap';

import { LocalStorageService, UIAction } from '@app/core';

import { SETTINGS_KEY, SETTINGS_CHANGE_THEME } from './settings.reducer';

@Injectable()
export class SettingsEffects {
  constructor(
    private actions$: Actions<UIAction>,
    private localStorageService: LocalStorageService
  ) { }

  @Effect({ dispatch: false })
  persistThemeSettings(): Observable<UIAction> {
    return this.actions$.ofType(SETTINGS_CHANGE_THEME).pipe(
      tap(action =>
        this.localStorageService.setItem(SETTINGS_KEY, {
          theme: action.payload
        })
      )
    );
  }
}
