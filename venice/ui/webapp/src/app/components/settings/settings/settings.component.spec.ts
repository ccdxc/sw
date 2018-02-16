import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { By } from '@angular/platform-browser';
import { DebugElement } from '@angular/core';
import { RouterTestingModule } from '@angular/router/testing';
import { FormsModule } from '@angular/forms';
import { CoreModule } from '@app/core';
import { MaterialdesignModule } from '@lib/materialdesign.module';

import { AuthService } from '@app/services/auth.service';
import { ControllerService } from '@app/services/controller.service';

import { SettingsComponent } from './settings.component';
import { Component } from '@angular/core';
@Component( {
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
          ControllerService
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
