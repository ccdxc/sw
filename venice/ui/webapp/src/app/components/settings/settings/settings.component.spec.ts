import { Component } from '@angular/core';
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { FormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { CoreModule } from '@app/core';
import { AuthService } from '@app/services/auth.service';
import { ControllerService } from '@app/services/controller.service';
import { MaterialdesignModule } from '@lib/materialdesign.module';

import { SettingsComponent } from './settings.component';

@Component({
  template: ''
})
class DummyComponent { }

describe('SettingsComponent', () => {
  let component: SettingsComponent;
  let fixture: ComponentFixture<SettingsComponent>;

  beforeEach(
    async(() => {
      TestBed.configureTestingModule({
        imports: [
          RouterTestingModule.withRoutes([
            { path: 'login', component: DummyComponent }
          ]),
          NoopAnimationsModule,
          RouterTestingModule,
          CoreModule,
          FormsModule,
          MaterialdesignModule,
        ],
        providers: [
          AuthService,
          ControllerService,
          MatIconRegistry
        ],
        declarations: [SettingsComponent, DummyComponent]
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(SettingsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should be created', () => {
    expect(component).toBeTruthy();
  });
});
