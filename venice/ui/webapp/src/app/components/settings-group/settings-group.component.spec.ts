/**-----
 Angular imports
 ------------------*/
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { Component } from '@angular/core';

/**-----
 Venice UI -  imports
 ------------------*/
import { SettingsGroupComponent } from './settings-group.component';
import { ControllerService } from '@app/services/controller.service';

/**-----
 Third-parties imports
 ------------------*/
import { MatIconRegistry } from '@angular/material';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';

@Component({
  template: ''
})
class DummyComponent { }

describe('SettingsGroupComponent', () => {
  let component: SettingsGroupComponent;
  let fixture: ComponentFixture<SettingsGroupComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [
        SettingsGroupComponent,
        DummyComponent
      ],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        HttpClientTestingModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        MatIconRegistry
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(SettingsGroupComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
