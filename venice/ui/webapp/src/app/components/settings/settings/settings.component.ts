import { Component, OnInit, OnDestroy } from '@angular/core';
import { Store } from '@ngrx/store';
import { Subject } from 'rxjs/Subject';
import { takeUntil } from 'rxjs/operators/takeUntil';

import { selectorSettings, actionChangeTheme } from '../settings.reducer';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { ControllerService } from '@app/services/controller.service';
import { Eventtypes} from '@app/enum/eventtypes.enum';
import { BaseComponent } from '@app/components/base/base.component';

@Component({
  selector: 'app-settings',
  templateUrl: './settings.component.html',
  styleUrls: ['./settings.component.scss']
})
export class SettingsComponent extends BaseComponent implements OnInit, OnDestroy {
  private unsubscribe$: Subject<void> = new Subject<void>();
  theme: string;

  themes = [
    { value: 'DEFAULT-THEME', label: 'Default' },
    { value: 'LIGHT-THEME', label: 'Light' },
    { value: 'BLACK-THEME', label: 'Black' }
  ];

  constructor(private store: Store<any>, protected _controllerService: ControllerService) {
    super(_controllerService);
    store
      .select(selectorSettings)
      .pipe(takeUntil(this.unsubscribe$))
      .subscribe(({ theme }) => (this.theme = theme));
  }

  ngOnInit() {
    if (!this._controllerService.isUserLogin()) {
      this._controllerService.publish(Eventtypes.NOT_YET_LOGIN, {});
    } else {
      const self = this;
      this._controllerService.publish(Eventtypes.COMPONENT_INIT, {
        'component': 'SettingsComponent', 'state':
          Eventtypes.COMPONENT_INIT
      });

    }
  }

  ngOnDestroy(): void {
    this.unsubscribe$.next();
    this.unsubscribe$.complete();
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, {
      'component': 'SettingsComponent', 'state':
        Eventtypes.COMPONENT_DESTROY
    });
  }

  /**
   * Overide super's API
   * It will return this Component name
   */
  getClassName(): string {
    return this.constructor.name;
  }

  onThemeSelect({ value }) {
    this.store.dispatch(actionChangeTheme(value));
  }
}
