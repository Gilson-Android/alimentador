# 7. Antes de viajar

Alimentador automático é o tipo de coisa que só falha quando você não está lá.
Este checklist existe para isso.

## Duas semanas antes

- [ ] Deixe o alimentador **rodando de verdade** por 7 dias, com você em casa.
      É o único teste que vale. Confira no *Histórico* se todos os horários
      dispararam.
- [ ] Veja se sobra ração no fundo do aquário depois das refeições. Se sobra,
      **diminua** os passos por porção.
- [ ] Cheque se o motor está esquentando muito. Morno é normal; quente demais
      significa passo rápido demais — aumente os µs por passo.

## Uma semana antes

- [ ] Troque a ração do funil por ração nova e seca.
- [ ] Sachê de sílica gel novo.
- [ ] Desmonte a calha e limpe (é só puxar). Pó de ração acumula ali.
- [ ] Verifique se a rosca gira livre: *Manutenção → Avançar*.
- [ ] Confirme o `Máx. porções/dia` — deixe justo, não generoso.
- [ ] Faça um teste de queda de energia: **desligue a fonte no meio de um
      horário programado e ligue 10 minutos depois.** Tem que alimentar ao voltar.

## Um dia antes

- [ ] Bot do Telegram respondendo `/status` e `/foto` do **4G do celular**
      (não do Wi-Fi de casa — precisa ser um teste real de acesso externo).
- [ ] Bateria/nobreak do roteador, se tiver.
- [ ] Reserve o IP do alimentador no roteador (DHCP estático), para não perder
      o acesso se o roteador reiniciar.
- [ ] Anote o IP e a senha da interface em algum lugar acessível na viagem.
- [ ] Câmera enquadrando a superfície da água e a saída da calha — assim você
      vê tanto os peixes quanto se a ração está caindo.
- [ ] Encha o funil só com o necessário para o período + 1 semana.

## Plano B (importante)

Nenhum equipamento caseiro tem 100% de confiabilidade. Combine com uma
segunda camada:

1. **Uma pessoa de confiança com a chave** — e avise que existe um alimentador
   automático, para ela não alimentar em dobro. Deixe ração porcionada em
   saquinhos, um por dia, e peça para usar **só se você pedir**.
2. **Jejum é seguro.** Peixe tropical adulto saudável passa tranquilamente
   5–7 dias sem comer. Se a viagem é de até uma semana, o risco de morrer de
   fome é muito menor que o de morrer por excesso de comida ou por pico de
   amônia. Configure com parcimônia.
3. Se a viagem for longa (>3 semanas), a manutenção da água é um problema
   maior que a comida: troca parcial de água é insubstituível.

## O que monitorar durante a viagem

- Uma foto por dia (`/foto`) já mostra muito: água turva, ração acumulada no
  fundo, peixe boiando, nível do funil.
- Se ligar *Avisar a cada alimentação*, a ausência de mensagem no horário é
  o sinal de alarme: pode ser Wi-Fi, energia ou entupimento.
- Com sensor de grãos ligado, você recebe aviso explícito de "não caiu ração".

## Se der problema e você estiver longe

| Sintoma | Provável causa | O que fazer à distância |
|---|---|---|
| Não responde nada | Falta de energia ou Wi-Fi | Peça para alguém reiniciar a fonte e o roteador |
| Responde mas "não caiu ração" | Rosca entupida / ração empedrada | Mande `/alimentar` algumas vezes; se tiver acesso à interface, use Recuar/Avançar |
| Alimentou mas o peixe não come | Ração no lugar errado ou peixe doente | Foto + pedir ajuda presencial |
| Alimentou demais | Limite alto demais | Baixe `Máx. porções/dia` para 1 e peça sifonagem |
