import { HttpClientTestingModule } from '@angular/common/http/testing';
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { FormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { RouterTestingModule } from '@angular/router/testing';
import { CoreModule } from '@app/core';

import { AuthService } from '../../services/auth.service';
import { ControllerService } from '../../services/controller.service';
import { LoginComponent } from './login.component';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { MessageService } from 'primeng/primeng';

describe('LoginComponent', () => {
  let component: LoginComponent;
  let fixture: ComponentFixture<LoginComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [LoginComponent],
      imports: [
        RouterTestingModule,
        FormsModule,
        HttpClientTestingModule,
        CoreModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        AuthService,
        MatIconRegistry,
        UIConfigsService,
        MessageService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(LoginComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
