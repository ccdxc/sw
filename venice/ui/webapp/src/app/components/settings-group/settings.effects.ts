import { Injectable } from '@angular/core';
import { Actions, Effect, ofType } from '@ngrx/effects';
import { Observable } from 'rxjs';
import { tap } from 'rxjs/operators';

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
    return this.actions$.pipe(
      ofType(SETTINGS_CHANGE_THEME),
      tap(action =>
        this.localStorageService.setItem(SETTINGS_KEY, {
          theme: action.payload
        })
      )
    );
  }
}
