import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { MaterialdesignModule } from '@lib/materialdesign.module';
import { ModalitemComponent } from './modalitem.component';

describe('ModalitemComponent', () => {
  let component: ModalitemComponent;
  let fixture: ComponentFixture<ModalitemComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ModalitemComponent ],
      imports: [MaterialdesignModule, ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ModalitemComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
